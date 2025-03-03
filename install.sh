#!/usr/bin/env bash

# Ensure script runs in Bash or Zsh
if [[ -z "$BASH_VERSION" && -z "$ZSH_VERSION" ]]; then
  echo "❌ This script requires Bash or Zsh to run."
  exit 1
fi

# Function to handle errors
handle_error() {
    echo "❌ Error: $1"
    exit 1
}

trap 'echo "❌ Error on line $LINENO"' ERR

# Detect script directory
find_script_directory() {
    local dir
    dir="$(dirname "$(realpath "$0")")"
    echo "$dir"
}

SCRIPT_DIR="$(find_script_directory)"

# Check for required file
if [[ ! -f "$SCRIPT_DIR/.here" ]]; then
    handle_error ".here file not found in script directory ($SCRIPT_DIR)"
fi

# Define paths
UTILITIES_DIR="$SCRIPT_DIR/utilities"
LOCAL_INSTALL_DIR="$SCRIPT_DIR/.local"
BIN_DIR="$LOCAL_INSTALL_DIR/bin"
LIB_DIR="$LOCAL_INSTALL_DIR/lib"
COMMANDS_DIR="$SCRIPT_DIR/commands"
CUSTOM_BASHRC="$COMMANDS_DIR/.bashrc"
OLLAMA_BIN="$BIN_DIR/ollama"

# Ensure necessary directories exist
mkdir -p "$UTILITIES_DIR" "$BIN_DIR" "$LIB_DIR" "$COMMANDS_DIR"
rm -f "$CUSTOM_BASHRC" &&
touch "$CUSTOM_BASHRC"

# Detect shell configuration file
if [[ -n "$BASH_VERSION" ]]; then
  SHELL_RC="$HOME/.bashrc"
elif [[ -n "$ZSH_VERSION" ]]; then
  SHELL_RC="$HOME/.zshrc"
fi

# Step 2: Download Ollama if not omitted
if [[ "$1" != "--omit_download_ollama" ]]; then
    if [[ ! -f "ollama-linux.tgz" ]]; then
        echo "Downloading Ollama..."
        wget https://ollama.com/download/ollama-linux-amd64.tgz -O ollama-linux.tgz || handle_error "Failed to download Ollama."
    fi
fi

# Extract Ollama if necessary
if [[ -f "ollama-linux.tgz" ]]; then
    echo "Extracting Ollama..."
    tar -xvzf ollama-linux.tgz -C "$LOCAL_INSTALL_DIR" || handle_error "Failed to extract Ollama."
else
    handle_error "ollama-linux.tgz not found please run the command without --omit_download_ollama"
fi

# Clone and build whisper.cpp
WHISPER_DIR="$UTILITIES_DIR/whisper.cpp"
if [[ ! -d "$WHISPER_DIR" ]]; then
    echo "Cloning whisper.cpp repository..."
    git clone https://github.com/ggerganov/whisper.cpp.git "$WHISPER_DIR" || handle_error "Failed to clone whisper.cpp."

    cd "$WHISPER_DIR" || handle_error "whisper.cpp directory not found."
    mkdir build && cd build
    cmake .. || handle_error "CMake configuration failed."
    make -j$(nproc) || handle_error "Build failed."
    cd "$SCRIPT_DIR"
else
    echo "whisper.cpp already exists. Skipping clone and build."
fi

# Download the whisper.cpp model
MODEL_DIR="$WHISPER_DIR/models"
MODEL_FILE="$MODEL_DIR/ggml-base.bin"
mkdir -p "$MODEL_DIR"
if [[ ! -f "$MODEL_FILE" ]]; then
    echo "Downloading whisper.cpp model..."
    wget -P "$MODEL_DIR" https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin || handle_error "Failed to download model."
else
    echo "whisper.cpp model already exists."
fi

# Install 'aplay' locally if missing
if ! command -v aplay &> /dev/null; then
    echo "Installing alsa-utils locally..."
    cd "$UTILITIES_DIR"
    git clone https://git.kernel.org/pub/scm/linux/kernel/git/tiwai/alsa-utils.git || handle_error "Failed to clone alsa-utils."
    cd alsa-utils
    ./configure --prefix="$LOCAL_INSTALL_DIR" && make -j$(nproc) && make install || handle_error "Failed to install alsa-utils."
    cd "$SCRIPT_DIR"
fi

# Install eSpeak NG if missing, or just set alias
if ! command -v espeak &> /dev/null; then
    echo "Installing eSpeak NG..."
    cd "$UTILITIES_DIR"
    if [[ ! -d "espeak-ng" ]]; then
        git clone https://github.com/espeak-ng/espeak-ng.git || handle_error "Failed to clone espeak-ng."
    fi
    cd espeak-ng
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL_DIR" || handle_error "CMake failed."
    make -j$(nproc) && make install || handle_error "Build failed."
    cd "$SCRIPT_DIR"
else
    echo "✔️ eSpeak is already installed. Skipping installation."
fi

# Update shell profile
if ! grep -q "$BIN_DIR" "$CUSTOM_BASHRC"; then
    echo "export PATH=$BIN_DIR:\$PATH" >> "$CUSTOM_BASHRC"
    echo "export LD_LIBRARY_PATH=$LIB_DIR:\$LD_LIBRARY_PATH" >> "$CUSTOM_BASHRC"
    echo "alias espeak='$BIN_DIR/espeak-ng'" >> "$CUSTOM_BASHRC"
    echo "alias ollama='$BIN_DIR/ollama'" >> "$CUSTOM_BASHRC"

fi

echo "✅ Please run: source $CUSTOM_BASHRC to apply changes."



trap - ERR
echo "🎉 Installation completed successfully!"