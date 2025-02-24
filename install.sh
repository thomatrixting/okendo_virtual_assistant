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


