#!/bin/bash
# Uso: ./setup.sh /path/to/root [--recompile]
# Ejemplo: ./setup.sh "$(pwd)" --recompile
# use this command ./setup.sh "$(pwd)" -r && source "$(pwd)/commands/.bashrc"

# Verify provided arguments
if [ -z "$1" ]; then
    echo "Error: No se proporcionó directorio raíz."
    echo "Uso: ./setup.sh /path/to/root [--recompile]"
    exit 1
fi

ROOT_DIR="$1"
RECOMPILE=false  # Default: No recompilation

if [ "$2" == "--recompile" ] || [ "$2" == "-r" ]; then
    RECOMPILE=true
fi

if [ ! -d "$ROOT_DIR" ]; then
    echo "Error: El directorio raíz proporcionado no existe."
    exit 1
fi

# Create commands and logs directories 
COMMANDS_DIR="$ROOT_DIR/commands"
mkdir -p "$COMMANDS_DIR"

LOG_DIR="$ROOT_DIR/logs"
mkdir -p "$LOG_DIR"

# Define setup log file and clear previous logs
LOG_FILE="$LOG_DIR/setup.log"
rm -rf "$LOG_DIR"/*.log

# Redirect all output to log file AND terminal (testing)
#exec > >(tee -a "$LOG_FILE") 2>&1
 exec > "$LOG_FILE" 2>&1 #Uncomment this line to send all to log file only

# Function to handle errors with support for custom messages
handle_error() {
    local line_number="$1"
    shift
    local custom_message="$*"
    local failed_command

    # Detect shell and get the last executed command
    if [ -n "$BASH_VERSION" ]; then
        failed_command="$BASH_COMMAND"
    elif [ -n "$ZSH_VERSION" ]; then
        failed_command="$(fc -ln -1)"
    else
        failed_command="Unknown"
    fi
    
    if [ -n "$custom_message" ]; then
        echo "⚠️  Error en la línea $line_number: $custom_message" >&2
    else
        echo "⚠️  Error en la línea $line_number al ejecutar: '$failed_command'" >&2
    fi

    echo "Revisa el archivo log: $LOG_FILE" >&2
    exit 1
}

# Trap for automatic error handling (passes the line number)
trap 'handle_error $LINENO' ERR

UTILITIES_DIR="$ROOT_DIR/utilities"
LOCAL_INSTALL_DIR="$ROOT_DIR/.local"
BIN_DIR="$LOCAL_INSTALL_DIR/bin"
LIB_DIR="$LOCAL_INSTALL_DIR/lib"
CUSTOM_BASHRC="$COMMANDS_DIR/.bashrc"
OLLAMA_BIN="$BIN_DIR/ollama"

# Define bashrc path and ensure it exists (COMMANDS_DIR already exists)
BASHRC_FILE="$ROOT_DIR/commands/.bashrc"
touch "$BASHRC_FILE"

# Function to add exports safely and apply them immediately
add_export() {
    local var="$1"
    local value="$2"

    # Check if the variable is already in the bashrc file
    if ! grep -q "^export $var=" "$BASHRC_FILE"; then
        echo "export $var=\"$value\"" >> "$BASHRC_FILE"
        echo "Export '$var' added to $BASHRC_FILE."
    else
        echo "Export '$var' already exists in $BASHRC_FILE. Skipping."
    fi

    # Apply the export immediately
    export "$var"="$value"
    echo "Export '$var' applied to current session."
}

# Add binary to PATH
add_export "PATH" "$BIN_DIR:$COMMANDS_DIR:$PATH"

# Stop Ollama server if running
if lsof -i :11434 &> /dev/null; then
    echo "El puerto 11434 ya está en uso. Deteniendo el servidor Ollama..."
    pkill -f ollama || echo "Advertencia: No se pudo detener algún proceso de Ollama."
fi

# Start Ollama server
add_export "OLLAMA_LOAD_TIMEOUT" "10m"
echo "Iniciando servidor de Ollama..."
ollama serve >> "$LOG_DIR/ollama_server.log" 2>&1 &

sleep 5

# Pull model
echo "Descargando el modelo deepseek-coder..."
ollama pull deepseek-coder 2>&1 | tee "$LOG_DIR/pull_log.txt" || handle_error "Error al descargar el modelo."

# Generate models
ollama create fast_response_assistant -f "$ROOT_DIR/utilities/models/fast_response_MODELFILE"
ollama create chat_response_assistant -f "$ROOT_DIR/utilities/models/chat_response_MODELFILE"
ollama create chat_response_unrestricted -f "$ROOT_DIR/utilities/models/unrestricted_chat_response_MODELFILE"

# Copy example files (if recompiling)
if [ "$RECOMPILE" = true ]; then
    echo "Recompilación activada. Copiando archivos de código fuente..."
    for file in "ask_the_model.cpp" "speak_with_the_model.cpp" "opcions.json" "historial_test.json" "OVA.cpp" "Makefile_OVA"; do
        if [ -f "$ROOT_DIR/examples/$file" ]; then
            cp "$ROOT_DIR/examples/$file" "$COMMANDS_DIR/"
        else
            echo "Advertencia: El archivo $file no existe, omitiendo copia."
        fi
    done
else
    echo "Recompilación omitida. Se usarán ejecutables existentes si están disponibles."
    for exe in "amfq.out" "chat.out" "OVA.out"; do
        if [ ! -f "$COMMANDS_DIR/$exe" ]; then
            echo "⚠️⚠️⚠️Advertencia: El ejecutable $exe no existe. Puede necesitar recompilar."
        fi
    done
fi

# Compile C++ files if recompiling
if [ "$RECOMPILE" = true ]; then
    echo "Compilando los archivos C++..."
    
    if g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/ask_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/amfq.out"; then
        echo "Compilación de ask_the_model.cpp exitosa."
    else
        handle_error "Fallo la compilación de ask_the_model.cpp."
    fi

    if g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/speak_with_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/chat.out" -g; then
        echo "Compilación de speak_with_the_model.cpp exitosa."
    else
        handle_error "Fallo la compilación de speak_with_the_model.cpp."
    fi

    # Compile OVA.cpp using Makefile_OVA
    echo "Compilando OVA.cpp usando Makefile_OVA..."
    if make -f "$COMMANDS_DIR/Makefile_OVA" -C "$COMMANDS_DIR"; then
        echo "Compilación de OVA.cpp exitosa."
    else
        handle_error "Fallo la compilación de OVA.cpp."
    fi
else
    echo "Recompilación omitida. Usando ejecutables existentes."
fi

# Configure LD_LIBRARY_PATH
add_export "LD_LIBRARY_PATH" "$ROOT_DIR/utilities/whisper.cpp/build/src:$LD_LIBRARY_PATH"
echo "Whisper.cpp setup successful!"

# Configure aliases
ALIAS_FILE="$COMMANDS_DIR/.bash_aliases"
if [ ! -f "$ALIAS_FILE" ]; then
    ALIAS_FILE="$BASHRC_FILE"
fi

add_alias() {
    local alias_name="$1"
    local alias_command="$2"
    if ! grep -q "^alias $alias_name=" "$ALIAS_FILE"; then
        echo "alias $alias_name=\"$alias_command\"" >> "$ALIAS_FILE"
        echo "Alias '$alias_name' added to $ALIAS_FILE."
    else
        echo "Alias '$alias_name' already exists in $ALIAS_FILE. Skipping."
    fi
}

add_alias "amfq" "$COMMANDS_DIR/amfq.out"
add_alias "chat" "$COMMANDS_DIR/chat.out"
add_alias "make" "make -f Makefile_OVA"

OVA_WRAPPER="$COMMANDS_DIR/ova.sh"

# Create the wrapper script
cat > "$OVA_WRAPPER" <<EOL
#!/bin/bash
cd "$COMMANDS_DIR" || exit 1
./OVA.out "\$@"
EOL

# Make the script executable
chmod +x "$OVA_WRAPPER"
add_alias "ova" "$COMMANDS_DIR/ova.sh"

echo "Aliases checked and updated in $ALIAS_FILE. Execute 'source $BASHRC_FILE' to activate them now."
echo "¡Configuración completada exitosamente! Los comandos 'amfq', 'chat' y 'ova' están disponibles en la terminal." > /dev/tty

exit 0