#!/bin/bash
# Uso: . ./restart_server.sh /path/to/root
# Ejemplo: . ./restart_server.sh "$(pwd)"  (ejecutar en la raíz del proyecto)

# Función para manejar errores de forma centralizada
handle_error() {
    echo "Error: $1"
    return 1
}

# Opcional: activar trampa para errores inesperados (no aborta inmediatamente, pero informa)
trap 'echo "Error on line "' ERR

# Verificar que se haya proporcionado el directorio raíz
if [ -z "$1" ]; then
    echo "Error: No se proporcionó directorio raíz."
    echo "Uso: . ./setup.sh /path/to/root"
    return 1
fi

ROOT_DIR="$1"

# Verificar que el directorio raíz exista
if [ ! -d "$ROOT_DIR" ]; then
    echo "Error: El directorio raíz proporcionado no existe."
    return 1
fi

# Agregar el binario de ollama al PATH usando el directorio raíz
export PATH="$ROOT_DIR/bin:$PATH"

# Crear el directorio de logs si no existe
LOG_DIR="$ROOT_DIR/logs"
if [ ! -d "$LOG_DIR" ]; then
    echo "Directorio de logs no encontrado. Creando $LOG_DIR..."
    mkdir -p "$LOG_DIR" || handle_error "No se pudo crear el directorio de logs."
fi

# Verificar si el puerto 11434 está en uso
if lsof -i :11434 &> /dev/null; then
    echo "El puerto 11434 ya está en uso. Deteniendo el servidor ollama existente..."
    pkill -f ollama || echo "Advertencia: No se pudo detener algún proceso de ollama."
    echo "Servidor ollama detenido."
fi

# Iniciar el servidor ollama en segundo plano y redirigir los logs
echo "Iniciando servidor de ollama..."
export OLLAMA_LOAD_TIMEOUT=10m

ollama serve >> "$LOG_DIR/ollama_server.log" 2>&1 &
if [ $? -ne 0 ]; then
    handle_error "No se pudo iniciar el servidor ollama."
fi

sleep 5

# Descargar el modelo deepseek-coder y verificar errores de conexión
echo "Descargando el modelo deepseek-coder de Ollama..."
ollama pull deepseek-coder 2>&1 | tee "$LOG_DIR/pull_log.txt"
if [ $? -ne 0 ]; then
    echo "Error detectado: La aplicación Ollama no está corriendo. Verifica su estado y vuelve a ejecutar la configuración."
    return 1
fi

# Generate models
ollama create fast_response_assistant -f "$ROOT_DIR/utilities/models/fast_response_MODELFILE"
ollama create chat_response_assistant -f "$ROOT_DIR/utilities/models/chat_response_MODELFILE"
ollama create chat_response_unrestricted -f "$ROOT_DIR/utilities/models/unrestricted_chat_response_MODELFILE"