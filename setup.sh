#!/bin/bash
# Uso: ./setup.sh /path/to/root [--recompile]
# Ejemplo: ./setup.sh "$(pwd)" --recompile
# use this comand ./setup.sh "$(pwd)" -r && source "$(pwd)/commands/.bashrc"


# Function to handle errors
handle_error() {
    echo "Error: $1"
    exit 1
}

# Opcional: activar trampa para errores inesperados (no aborta inmediatamente, pero informa)
trap 'echo "Error on line $LINENO"' ERR


# Verificar que se haya proporcionado el directorio raíz
# Verify provided arguments
if [ -z "$1" ]; then
    echo "Error: No se proporcionó directorio raíz."
    echo "Uso: ./setup.sh /path/to/root [--recompile]"
    exit 1
fi

ROOT_DIR="$1"
RECOMPILE=false  # Default: No recompilation

# Handle --recompile flag
if [ "$2" == "--recompile" ] || [ "$2" == "-r" ]; then
    RECOMPILE=true
fi

# Ensure root directory exists
if [ ! -d "$ROOT_DIR" ]; then
    echo "Error: El directorio raíz proporcionado no existe."
    exit 1
fi

# Add binary to PATH
export PATH="$ROOT_DIR/bin:$PATH"

# Create logs directory if missing
LOG_DIR="$ROOT_DIR/logs"
mkdir -p "$LOG_DIR"
rm -rf "$LOG_DIR"/*.log  # Clear logs


# Stop Ollama server if running
if lsof -i :11434 &> /dev/null; then
    echo "El puerto 11434 ya está en uso. Deteniendo el servidor Ollama..."
    pkill -f ollama || echo "Advertencia: No se pudo detener algún proceso de Ollama."
fi

# Start Ollama server
echo "Iniciando servidor de Ollama..."
export OLLAMA_LOAD_TIMEOUT=10m
ollama serve >> "$LOG_DIR/ollama_server.log" 2>&1 &

# Wait before pulling the model
sleep 5

# Pull model
echo "Descargando el modelo deepseek-coder..."
ollama pull deepseek-coder 2>&1 | tee "$LOG_DIR/pull_log.txt" || handle_error "Error al descargar el modelo."

# Generate models
ollama create fast_response_assistant -f "$ROOT_DIR/utilities/models/fast_response_MODELFILE"
ollama create chat_response_assistant -f "$ROOT_DIR/utilities/models/chat_response_MODELFILE"
ollama create chat_response_unrestricted -f "$ROOT_DIR/utilities/models/unrestricted_chat_response_MODELFILE"


# Ensure commands directory exists
COMMANDS_DIR="$ROOT_DIR/commands"
mkdir -p "$COMMANDS_DIR"

# Copy example files (if recompiling)
if [ "$RECOMPILE" = true ]; then
    echo "Recompilación activada. Copiando archivos de código fuente..."
    for file in "ask_the_model.cpp" "speak_with_the_model.cpp" "opcions.json" "historial_test.json"; do
        if [ -f "$ROOT_DIR/examples/$file" ]; then
            cp "$ROOT_DIR/examples/$file" "$COMMANDS_DIR/"
        else
            echo "Advertencia: El archivo $file no existe, omitiendo copia."
        fi
    done
else
    echo "Recompilación omitida. Se usarán ejecutables existentes si están disponibles."

    # Check if executables exist
    for exe in "amfq.out" "chat.out"; do
        if [ ! -f "$COMMANDS_DIR/$exe" ]; then
            echo "Advertencia: El ejecutable $exe no existe. Puede necesitar recompilar."
        fi
    done
fi

# Compile C++ files if recompiling
if [ "$RECOMPILE" = true ]; then
    echo "Compilando los archivos C++..."
    
    # Compile `ask_the_model.cpp`
    if g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/ask_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/amfq.out"; then
        echo "Compilación de ask_the_model.cpp exitosa."
    else
        handle_error "Fallo la compilación de ask_the_model.cpp."
    fi

    # Compile `speak_with_the_model.cpp`
    if g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/speak_with_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/chat.out" -g; then
        echo "Compilación de speak_with_the_model.cpp exitosa."
    else
        handle_error "Fallo la compilación de speak_with_the_model.cpp."
    fi
else
    echo "Recompilación omitida. Usando ejecutables existentes."
fi

# Add commands to PATH
export PATH="$COMMANDS_DIR:$PATH"


#Configure LD_LIBRARY_PATH for whisper.cpp shared library
echo "Configuring LD_LIBRARY_PATH for whisper.cpp..."
export LD_LIBRARY_PATH="$ROOT_DIR/utilities/whisper.cpp/build/src:$LD_LIBRARY_PATH"

echo "Whisper.cpp setup succesfull!"


# ✅ Make aliases persist by adding them to ~/.bashrc or ~/.bash_aliases
ALIAS_FILE="$COMMANDS_DIR/.bash_aliases"
if [ ! -f "$ALIAS_FILE" ]; then
    ALIAS_FILE="$COMMANDS_DIR/.bashrc"  # Fallback to .bashrc if .bash_aliases does not exist
fi

echo "alias amfq=\"$COMMANDS_DIR/amfq.out\"" >> "$ALIAS_FILE"
echo "alias chat=\"$COMMANDS_DIR/chat.out\"" >> "$ALIAS_FILE"
echo "Aliases guardados en $ALIAS_FILE. Ejecute 'source $ALIAS_FILE' para activarlos ahora."

trap - ERR
# ✅ Automatically reload aliases (optional)
source "$ALIAS_FILE"

echo "¡Configuración completada exitosamente! Los comandos 'amfq' y 'chat' están disponibles en la terminal."
exit 0
