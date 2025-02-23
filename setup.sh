#!/bin/bash

# Usage: . ./setup.sh /path/to/root
# . ./setup.sh $(pwd) in case of being on the root of the project
# Check if root directory is provided as an argument
if [ -z "$1" ]; then
    echo "Error: No root directory provided."
    echo "Usage: . ./setup.sh /path/to/root"
    exit 1
fi

ROOT_DIR="$1"

# Check if the provided root directory exists
if [ ! -d "$ROOT_DIR" ]; then
    echo "Error: Provided root directory does not exist."
    exit 1
fi

# Add the ollama binary to the PATH using the root directory
export PATH="$ROOT_DIR/bin:$PATH"

# Create logs directory if it doesn't exist
LOG_DIR="$ROOT_DIR/logs"
if [ ! -d "$LOG_DIR" ]; then
    echo "Logs directory not found. Creating $LOG_DIR..."
    mkdir -p "$LOG_DIR"
    if [ $? -ne 0 ]; then
        echo "Failed to create logs directory. Exiting."
        exit 1
    fi
fi

# Check if port 11434 is in use
if lsof -i :11434 &> /dev/null; then
    echo "Port 11434 is already in use. Stopping existing ollama server..."
    pkill -f ollama
    echo "Previous ollama server stopped."
fi

# Start the ollama server in the background and redirect logs to /logs
ollama serve >> "$LOG_DIR/ollama_server.log" 2>&1 &

# Pull the required model and check for connection error
echo "Pulling the deepseek-coder model from Ollama..."
ollama pull deepseek-coder 2>&1 | tee $LOG_DIR/pull_log.txt | grep -q "Error: could not connect to ollama app, is it running?"

# Check if the connection error occurred
if [ $? -eq 0 ]; then
    echo "Error detected: Ollama app is not running. Restarting setup..."
    exec "$0" "$ROOT_DIR"  # Re-run the script from the start with the same argument
    exit 0  # Skip the remaining commands
fi

# Create commands directory if it doesn't exist
COMMANDS_DIR="$ROOT_DIR/commands"
if [ ! -d "$COMMANDS_DIR" ]; then
    echo "Commands directory not found. Creating $COMMANDS_DIR..."
    mkdir -p "$COMMANDS_DIR"
    if [ $? -ne 0 ]; then
        echo "Failed to create commands directory. Exiting."
        exit 1
    fi
fi

# Copy example files into the commands directory
echo "Copying necessary files to the commands directory..."
cp "$ROOT_DIR/examples/ask_the_model.cpp" "$COMMANDS_DIR/"
cp "$ROOT_DIR/examples/speak_with_the_model.cpp" "$COMMANDS_DIR/"
cp "$ROOT_DIR/examples/opcions.json" "$COMMANDS_DIR/"
cp "$ROOT_DIR/examples/historial_test.json" "$COMMANDS_DIR/"

# Compile ask_the_model.cpp into amfq.out
echo "Compiling ask_the_model.cpp into amfq.out..."
g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/ask_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/amfq.out"
if [ $? -ne 0 ]; then
    echo "Failed to compile ask_the_model.cpp. Exiting."
    exit 1
fi

# Compile speak_with_the_model.cpp into chat.out
echo "Compiling speak_with_the_model.cpp into chat.out..."
g++ -std=c++17 -fsanitize=undefined "$COMMANDS_DIR/speak_with_the_model.cpp" "$ROOT_DIR/utilities/call_the_model.cpp" -o "$COMMANDS_DIR/chat.out" -g
if [ $? -ne 0 ]; then
    echo "Failed to compile speak_with_the_model.cpp. Exiting."
    exit 1
fi

# Add commands directory to PATH for the current session
export PATH="$COMMANDS_DIR:$PATH"
alias amfq="$COMMANDS_DIR/amfq.out"
alias chat="$COMMANDS_DIR/chat.out"

echo "Setup completed successfully! Commands 'amfq' and 'chat' are now available for this session."