#This commands can be used to install and build Whisper.cpp
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp
mkdir build && cd build
cmake ..
make -j$(nproc)

#Link to the model used in the program.
wget https://huggingface.co/ggerganov/whisper.cpp/blob/main/ggml-base.bin

mv ~/home/felipeg/downloads/ggml-base.bin ~/home/felipeg/whisper.cpp/models/

#Command used to configure the PATH of Whisper.cpp shared library.
export LD_LIBRARY_PATH=/home/felipeg/whisper.cpp/build/src:$LD_LIBRARY_PATH 

#Compilation line (need to adapt to new devices)
g++ microkey.cpp -I/home/felipeg/whisper.cpp/include -I/home/felipeg/whisper.cpp/ggml/include -L/home/felipeg/whisper.cpp/build/src -lwhisper -o microkey
