# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -fsanitize=undefined -g -Wl,-rpath,$(BUILD_DIR):$(GGML_BUILD_DIR)

# Directories
UTILS = ../utilities
WHISPER_DIR = $(UTILS)/whisper.cpp
GGML_DIR = $(WHISPER_DIR)/ggml
BUILD_DIR = $(WHISPER_DIR)/build/src
GGML_BUILD_DIR = $(WHISPER_DIR)/build/ggml/src

# Include and Library Paths
INCLUDES = -I$(WHISPER_DIR)/include -I$(GGML_DIR)/include
LIBS = -L$(BUILD_DIR) -lwhisper -L$(GGML_BUILD_DIR) -lggml -lggml-cpu -lggml-base -Wl,--no-as-needed

# Source Files
SRCS = OVA.cpp \
       $(UTILS)/call_the_model.cpp \
       $(UTILS)/transcriber.cpp \
       $(UTILS)/voicer.cpp

# Output Executable
TARGET = OVA.out

# Compilation Rule
$(TARGET): $(SRCS)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRCS) $(LIBS) -o $(TARGET)

# Clean Rule
clean:
	@rm -f $(TARGET)
