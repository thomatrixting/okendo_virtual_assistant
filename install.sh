wget https://ollama.com/download/ollama-linux-amd64.tgz -O ollama-linux.tgz
tar -xvzf ollama-linux.tgz
./bin/ollama serve & #TODO: make that set_up_calling_of_the_model open the server
./bin/ollama pull deepseek-r1:1.5b
