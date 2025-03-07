#include <chrono>
#include <poll.h>
#include <thread>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <filesystem>
#include "../utilities/transcriber.hpp"

// Global log file stream
std::ofstream whisperLogFile("../logs/whisper.log", std::ios::app);

// Custom logging callback function for whisper
void Transcriber::customWhisperLogCallback(ggml_log_level level, const char * text, void * user_data) {
    if (whisperLogFile.is_open()) {
        whisperLogFile << text;
        whisperLogFile.flush();
    }
}

// Constructor
Transcriber::Transcriber(const std::string &modelPath, const std::string &audioPath) {
    const std::string logDirectory = "../logs";
    const std::string logFilePath = logDirectory + "/whisper.log";

    std::filesystem::create_directories(logDirectory);
    std::ofstream logFile(logFilePath, std::ios::app);
    if (!logFile) {
        std::cerr << "Error opening log file!" << std::endl;
        exit(1);
    }

    std::streambuf *coutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(logFile.rdbuf());
    std::streambuf *cerrBuffer = std::cerr.rdbuf();
    std::cerr.rdbuf(logFile.rdbuf());  

    ParametersWhisper wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params(modelPath.c_str(), wparams);

    std::cout.rdbuf(coutBuffer);
    std::cerr.rdbuf(cerrBuffer);  

    if (!ctx) {
        std::cerr << " ❌ Error: Could not load Whisper model." << std::endl;
        exit(1);
    }

    audioFile = audioPath;
    recordCommand = "arecord -f S16_LE -r 16000 -c 1 audio.wav > /dev/null 2>&1 & echo $! > /tmp/arecord_pid";
    stopCommand = "if [ -f /tmp/arecord_pid ]; then kill $(cat /tmp/arecord_pid) && rm -f /tmp/arecord_pid; fi";
}

// Destroyer
Transcriber::~Transcriber() {
    whisper_free(ctx);
}

// Logging error and success messages from other functions
void Transcriber::logMsg(const std::string& message) {
    
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "transcriber.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << " ❌ Error: Could not open log file in " << logFilePath << std::endl;
    }
}

// Function to get the terminal configuration
void set_terminal_attributes(struct termios& oldt, struct termios& newt) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    DISABLE_CANONICAL_MODE(newt);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

// Function to restore the original configuration of the terminal
void restore_terminal_settings(struct termios& oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// Function to get the state of an key without blocking
int get_key() {
    return getchar();
}

// Function to handle keyboard input without blocking
bool keyboardhit() {
    struct termios oldt, newt;
    int oldf;
    set_terminal_attributes(oldt, newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    SET_NONBLOCKING_MODE();

    int ch = get_key();

    restore_terminal_settings(oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

// Start microphone recording
void Transcriber::start_microphone() {
    // Use the audioFile member to determine the file path.
    if (std::filesystem::exists(audioFile)) {
        std::filesystem::remove(audioFile);
        std::string errMsg = " ❌ Previous audio file removed: " + audioFile;
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
    }
    
    std::cout << " 🎤 Press 'R' to talk." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            printf("\b \b"); // Remove character from terminal
            if (key == 'r' || key == 'R') {
                std::cout << " 🎙️ Recording..." << std::endl;
                // Execute the recording command.
                if (system(recordCommand) != 0) {
                    std::string errMsg = " ❌ Error starting recording";
                    // perror(errMsg.c_str());
                    logMsg(errMsg);
                }
                
                // Wait until the audio file is created and has non-zero size.
                const int maxRetries = 10;
                int retries = 0;
                while ((!std::filesystem::exists(audioFile) ||
                        std::filesystem::file_size(audioFile) == 0) && retries < maxRetries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    retries++;
                }
                
                if (!std::filesystem::exists(audioFile) || std::filesystem::file_size(audioFile) == 0) {
                    std::string errMsg = "❌ Error: Failed to start recording. File: " + audioFile;
                    // std::cerr << errMsg << std::endl;
                    logMsg(errMsg);
                    return;
                } else {
                    std::string msg = "✅ Audio file created: " + audioFile 
                    + " (Size: " + std::to_string(std::filesystem::file_size(audioFile)) + " bytes)";
                    // std::cout << msg << std::endl;
                    logMsg(msg);
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Stop microphone recording
void Transcriber::stop_microphone() {
    std::cout << "Press 'S' to stop." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            printf("\b \b"); // Remove character from terminal
            if (key == 's' || key == 'S') {
                std::cout << "🛑 Stopping..." << std::endl;
                if (system(stopCommand) != 0) {
                    if (errno != 0) {
                    std::string errMsg = "❌ Error stopping recording";
                        // perror(errMsg.c_str());
                        logMsg(errMsg);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                
                // Verify the file exists after stopping the recording.
                if (std::filesystem::exists(audioFile)) {
                    std::string msg = "✅ Audio file found after stopping recording: " + audioFile +
                                      " (Size: " + std::to_string(std::filesystem::file_size(audioFile)) + " bytes)";
                    // std::cout << msg << std::endl;
                    logMsg(msg);
                } else {
                    std::string errMsg = "❌ Error: Audio file not found after stopping recording.";
                    // std::cerr << errMsg << std::endl;
                    logMsg(errMsg);
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

// Load WAV file and convert it to a normalized double vector
std::vector<float> Transcriber::load_audio(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::string errMsg = " ❌ Audio file does not exist: " + filename;
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }

    // Read the WAV header (assumed to be 44 bytes)
    char header[44];
    file.read(header, 44);
    if (file.gcount() < 44) {
        std::string errMsg = " ❌ Error: Insufficient WAV header.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }

    // Determine the size of the data section by using the total file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    std::streamoff dataSize = fileSize - static_cast<std::streamoff>(44); // remaining bytes after header
    if (dataSize <= 0) {
        std::string errMsg = " ❌ No audio data.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    if (dataSize % sizeof(int16_t) != 0) {
        std::string errMsg = " ❌ Audio data size not aligned to 16-bit samples.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    size_t numSamples = dataSize / sizeof(int16_t);

    // Return to the beginning of the data section
    file.seekg(44, std::ios::beg);
    std::vector<int16_t> samples(numSamples);
    file.read(reinterpret_cast<char*>(samples.data()), dataSize);
    if (static_cast<size_t>(file.gcount()) < numSamples * sizeof(int16_t)) {
        std::string errMsg = " ❌ Error reading audio samples.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    file.close();

    // Normalize samples to doubles in range [-1.0, 1.0)
    std::vector<float> audioData(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        audioData[i] = samples[i] / 32768.0f;
    }

    return audioData;
}


// Transcribe audio using the Whisper API
std::string Transcriber::transcribe_audio() {
    std::vector<float> audioData = load_audio(audioFile);
    if (audioData.empty()) {
        std::string errMsg = " ❌ Audio could not be loaded.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return "";
    }

    WhisperConfig params = whisper_create_parameters(WHISPER_SAMPLING_GREEDY);
    params.language = "en";
    params.print_progress = false;

    if (whisper_full(ctx, params, audioData.data(), audioData.size()) != 0) {
        std::string errMsg = " ❌ Error transcribing audio.";
        // std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return "";
    }

    int num_segments = whisper_num_segments(ctx);
    std::string transcript = "You: ";
    for (int i = 0; i < num_segments; ++i) {
        transcript += whisper_get_text_segment(ctx, i);
        transcript += " ";
    }

    return transcript;
}