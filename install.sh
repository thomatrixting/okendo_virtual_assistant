#!/bin/bash

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

# -------------------------------
# Step 5: Checks and install aplay locally (if not already present)
# -------------------------------

# 1. Check if 'aplay' is installed
if ! command -v aplay &> /dev/null; then
  echo "'aplay' not found. Attempting local installation of alsa-utils..."

  # 2. Install alsa-utils locally (this installs aplay)
  git clone https://git.kernel.org/pub/scm/linux/kernel/git/tiwai/alsa-utils.git
  cd alsa-utils
  ./configure --prefix=$HOME/.local
  make -j$(nproc)
  if ! make install; then
    echo "Warning: Failed to install 'aplay' (alsa-utils). You might not be able to play sound."
  else
    echo "Successfully installed 'aplay' locally."
  fi
  # Return to the previous directory
  cd ..
else
  echo "'aplay' is already installed. Skipping installation."
fi

# -------------------------------
# Step 6: Clone and build eSpeak NG (if not already present)
# -------------------------------

# 1. Check if eSpeak is already installed
if ! command -v espeak &> /dev/null; then
  echo "eSpeak not found. Proceeding with installation..."

  # 2. Cloning and checking out the release in the eSpeak NG repository                  
  git clone https://github.com/espeak-ng/espeak-ng.git
  cd espeak-ng
  git checkout tags/1.52.0  # Ensure this tag exists

  # 3. Create and build eSpeak NG
  mkdir -p build && cd build
  cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
  make -j$(nproc)
  if ! make install; then
    echo "make install failed, copying the binary manually..."
    mkdir -p $HOME/.local/bin
    cp ~/espeak-ng/build/src/espeak $HOME/.local/bin/
    chmod +x $HOME/.local/bin/espeak
  fi

  # 10. Add the local installation to your PATH and update LD_LIBRARY_PATH
  echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.zshrc
  echo 'export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH' >> ~/.zshrc
  source ~/.zshrc
else
  echo "eSpeak is already installed. Skipping installation."
fi

# 11. Test eSpeak with audio output
espeak "Hello, this is a test" --stdout | aplay
