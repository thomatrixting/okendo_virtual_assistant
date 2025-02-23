#!/bin/bash

# Step 2: Download Ollama
echo "Downloading Ollama..."
wget https://ollama.com/download/ollama-linux-amd64.tgz -O ollama-linux.tgz

if [ $? -ne 0 ]; then
    echo "Failed to download Ollama. Exiting."
    exit 1
fi

# Step 3: Extract the downloaded tarball
echo "Extracting Ollama..."
tar -xvzf ollama-linux.tgz

if [ $? -ne 0 ]; then
    echo "Failed to extract Ollama. Exiting."
    exit 1
fi


echo "Ollama installation and setup completed successfully!"
