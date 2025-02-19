export PATH="$(pwd)/bin:$PATH" #make ollama and comands on unitlitis work
ollama serve & 
ollama pull deepseek-coder