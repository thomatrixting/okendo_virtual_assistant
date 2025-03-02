#!/bin/bash

#!/bin/bash

# Exit on any error and print an error message
set -e
trap 'echo "❌ Error: Script failed at line $LINENO"; exit 1' ERR

# Ensure script runs in both Bash and Zsh
if [[ -z "$BASH_VERSION" && -z "$ZSH_VERSION" ]]; then
  echo "❌ This script requires Bash or Zsh to run."
  exit 1
fi

# Define installation directory
UTILITIES_DIR="$HOME/utilities"
LOCAL_INSTALL_DIR="$HOME/.local"
BIN_DIR="$LOCAL_INSTALL_DIR/bin"
LIB_DIR="$LOCAL_INSTALL_DIR/lib"

# Ensure utilities directory exists
mkdir -p "$UTILITIES_DIR"
mkdir -p "$BIN_DIR"
mkdir -p "$LIB_DIR"

# Detect shell configuration file (for PATH updates)
if [[ -n "$BASH_VERSION" ]]; then
  SHELL_RC="$HOME/.bashrc"
elif [[ -n "$ZSH_VERSION" ]]; then
  SHELL_RC="$HOME/.zshrc"
fi

#!/bin/bash

# Exit on any error and print an error message
set -e
trap 'echo "❌ Error: Script failed at line $LINENO"; exit 1' ERR

# -------------------------------
# Function: Detect Script Directory
# -------------------------------
find_script_directory() {
    local dir
    dir="$(dirname "$(realpath "$0")")"

# -------------------------------
# Step 1: Download Ollama
# -------------------------------
echo "Downloading Ollama..."
wget https://ollama.com/download/ollama-linux-amd64.tgz -O ollama-linux.tgz
if [ $? -ne 0 ]; then
    echo "Failed to download Ollama. Exiting."
    exit 1
fi

# -------------------------------
# Step 2: Extract the downloaded tarball
# -------------------------------
echo "Extracting Ollama..."
tar -xvzf ollama-linux.tgz
if [ $? -ne 0 ]; then
    echo "Failed to extract Ollama. Exiting."
    exit 1
fi

echo "Ollama installation and setup completed successfully!"

# -------------------------------
# Step 3: Clone and build whisper.cpp (install under utilities)
# -------------------------------
WHISPER_DIR="utilities/whisper.cpp"
if [ -d "$WHISPER_DIR" ]; then
    echo "whisper.cpp already exists in utilities. Skipping clone and build."
else
    echo "Cloning whisper.cpp repository into $WHISPER_DIR..."
    git clone https://github.com/ggerganov/whisper.cpp.git "$WHISPER_DIR"
    if [ $? -ne 0 ]; then
        echo "Failed to clone whisper.cpp repository. Exiting."
        exit 1
    fi

    cd "$WHISPER_DIR" || { echo "whisper.cpp directory not found. Exiting."; exit 1; }

    echo "Creating build directory..."
    mkdir build && cd build
    if [ $? -ne 0 ]; then
        echo "Failed to create or access the build directory. Exiting."
        exit 1
    fi

    echo "Configuring the project with CMake..."
    cmake ..
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed. Exiting."
        exit 1
    fi

    echo "Building whisper.cpp..."
    make -j$(nproc)
    if [ $? -ne 0 ]; then
        echo "Build failed. Exiting."
        exit 1
    fi

    # Return to the main directory
    cd ../../..
fi

# -------------------------------
# Step 4: Download the whisper.cpp model (if not already present)
# -------------------------------
MODEL_DIR="$WHISPER_DIR/models"
MODEL_FILE="ggml-base.bin"
if [ -f "$MODEL_DIR/$MODEL_FILE" ]; then
    echo "whisper.cpp model already exists. Skipping download."
else
    echo "Downloading the whisper.cpp model..."
    mkdir -p "$MODEL_DIR"
    wget -P "$MODEL_DIR" https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin
    if [ $? -ne 0 ]; then
        echo "Failed to download the whisper.cpp model. Exiting."
        exit 1
    fi
fi



    # Check if .here exists in the directory
    if [ ! -f "$dir/.here" ]; then
        echo "❌ Error: .here file not found in script directory ($dir)."
        exit 1
    fi

    echo "$dir"
}

# Get the script location
SCRIPT_DIR="$(find_script_directory)"

# Define paths based on script location
UTILITIES_DIR="$SCRIPT_DIR/utilities"
LOCAL_INSTALL_DIR="$SCRIPT_DIR/.local"
BIN_DIR="$LOCAL_INSTALL_DIR/bin"
LIB_DIR="$LOCAL_INSTALL_DIR/lib"
COMMANDS_DIR="$SCRIPT_DIR/commands"
CUSTOM_BASHRC="$COMMANDS_DIR/.bashrc"

# Ensure necessary directories exist
mkdir -p "$UTILITIES_DIR"
mkdir -p "$BIN_DIR"
mkdir -p "$LIB_DIR"
mkdir -p "$COMMANDS_DIR"

# Ensure the custom bashrc file exists
touch "$CUSTOM_BASHRC"

# -------------------------------
# Step 1: Checks and install 'aplay' locally (if not already present)
# -------------------------------

if ! command -v aplay &> /dev/null; then
  echo "🔧 'aplay' not found. Installing alsa-utils locally..."

  cd "$UTILITIES_DIR"
  if [ ! -d "alsa-utils" ]; then
    git clone https://git.kernel.org/pub/scm/linux/kernel/git/tiwai/alsa-utils.git
  fi
  cd alsa-utils

  ./configure --prefix="$LOCAL_INSTALL_DIR"
  make -j$(nproc)
  make install

  echo "✅ Successfully installed 'aplay'."
else
  echo "✔️ 'aplay' is already installed. Skipping installation."
fi

# -------------------------------
# Step 2: Clone and build eSpeak NG (if not already present)
# -------------------------------

if ! command -v espeak &> /dev/null; then
  echo "🔧 'eSpeak' not found. Proceeding with installation..."

  cd "$UTILITIES_DIR"
  if [ ! -d "espeak-ng" ]; then
    git clone https://github.com/espeak-ng/espeak-ng.git
  fi
  cd espeak-ng

  git checkout tags/1.52.0

  mkdir -p build && cd build
  cmake .. -DCMAKE_INSTALL_PREFIX="$LOCAL_INSTALL_DIR"
  make -j$(nproc)
  make install || {
    echo "⚠️ 'make install' failed, copying the binary manually..."
    cp src/espeak-ng "$BIN_DIR/espeak"
    chmod +x "$BIN_DIR/espeak"
  }

  if [ ! -f "$BIN_DIR/espeak" ]; then
    ln -s "$BIN_DIR/espeak-ng" "$BIN_DIR/espeak"
  fi

  echo "✅ Successfully installed 'eSpeak'."
else
  echo "✔️ 'eSpeak' is already installed. Skipping installation."
fi

# -------------------------------
# Step 3: Update the custom shell profile (commands/.bashrc)
# -------------------------------

if ! grep -q "$BIN_DIR" "$CUSTOM_BASHRC"; then
  echo "export PATH=$BIN_DIR:\$PATH" >> "$CUSTOM_BASHRC"
fi
if ! grep -q "$LIB_DIR" "$CUSTOM_BASHRC"; then
  echo "export LD_LIBRARY_PATH=$LIB_DIR:\$LD_LIBRARY_PATH" >> "$CUSTOM_BASHRC"
fi

# Inform user to source the custom bashrc
echo "✅ Please run: source $CUSTOM_BASHRC to apply changes."

# -------------------------------
# Step 4: Test eSpeak with audio output
# -------------------------------

echo "🔊 Testing eSpeak output..."
source "$CUSTOM_BASHRC"
espeak "Hello, this is a test" --stdout | aplay

echo "🎉 Installation completed successfully!"