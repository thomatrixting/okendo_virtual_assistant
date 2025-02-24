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
# Step 3: Clone and build whisper.cpp
# -------------------------------
echo "Cloning whisper.cpp repository..."
git clone https://github.com/ggerganov/whisper.cpp.git
if [ $? -ne 0 ]; then
    echo "Failed to clone whisper.cpp repository. Exiting."
    exit 1
fi

cd whisper.cpp || { echo "whisper.cpp directory not found. Exiting."; exit 1; }

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

# -------------------------------
# Step 4: Download the model used in the program
# -------------------------------
echo "Downloading the whisper.cpp model..."
sudo wget -P /home/felipeg/whisper.cpp/models https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin
if [ $? -ne 0 ]; then
    echo "Failed to download the whisper.cpp model. Exiting."
    exit 1
fi

# -------------------------------
# Step 5: Configure LD_LIBRARY_PATH for whisper.cpp shared library
# -------------------------------
echo "Configuring LD_LIBRARY_PATH for whisper.cpp..."
export LD_LIBRARY_PATH=/home/felipeg/whisper.cpp/build/src:$LD_LIBRARY_PATH

echo "Whisper.cpp installation and setup completed successfully!"
