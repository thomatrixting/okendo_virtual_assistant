wget https://ollama.com/download/ollama-linux-amd64.tgz -O ollama-linux.tgz
tar -xvzf ollama-linux.tgz
export PATH="$(pwd)/bin:$PATH" #make ollama and comands on unitlitis work
ollama serve & 
ollama pull deepseek-coder
