#This commands can be used to install and build Whisper.cpp
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp
mkdir build && cd build
cmake ..
make -j$(nproc)

#Link to the model used in the program.
sudo wget -P /home/felipeg/whisper.cpp/models https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin

#Command used to configure the PATH of Whisper.cpp shared library.
export LD_LIBRARY_PATH=/home/felipeg/whisper.cpp/build/src:$LD_LIBRARY_PATH 
