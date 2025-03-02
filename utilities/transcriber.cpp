#include "../utilities/transcriber.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <filesystem>
#include <poll.h>
#include <sstream>
#include "transcriber.hpp"

// Global log file stream
std::ofstream whisperLogFile("../logs/whisper.log", std::ios::app);

// Custom logging callback function for whisper
void customWhisperLogCallback(ggml_log_level level, const char * text, void * user_data) {
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

    whisper_log_set(customWhisperLogCallback, nullptr);
    ParametrosWhisper wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params(modelPath.c_str(), wparams);

    std::cout.rdbuf(coutBuffer);
    std::cerr.rdbuf(cerrBuffer);  

    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        exit(1);
    }

    audioFile = audioPath;     
    recordCommand = "arecord -f S16_LE -r 16000 -c 1 audio.wav > /dev/null 2>&1 & echo $! > /tmp/arecord_pid";
    stopCommand = "if [ -f /tmp/arecord_pid ]; then kill $(cat /tmp/arecord_pid) && rm -f /tmp/arecord_pid; fi";
}

// Destructor
Transcriber::~Transcriber() {
    whisper_free(ctx);
}

//Logging error and success messages from other functions
void logMsg(const std::string& message) {
    
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "transcriber.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "❌ Error: No se pudo abrir el archivo de registro en " << logFilePath << std::endl;
    }
}

// Función para obtener la configuración de la terminal
void set_terminal_attributes(struct termios& oldt, struct termios& newt) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    DISABLE_CANONICAL_MODE(newt);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

// Función para restaurar la configuración original de la terminal
void restore_terminal_settings(struct termios& oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// Función para obtener el estado de una tecla sin bloquear
int get_key() {
    return getchar();
}

// Función para gestionar la entrada de teclado sin bloqueo
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
        std::string errMsg = "❌ Archivo de audio anterior eliminado: " + audioFile;
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
    }
    
    std::cout << "🎤 Press 'R' to talk." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            printf("\b \b"); // Remove character from terminal
            if (key == 'r' || key == 'R') {
                std::cout << "🎙️ Recording..." << std::endl;
                // Execute the recording command.
                if (system(recordCommand) != 0) {
                    std::string errMsg = "❌ Error al iniciar la grabación";
                    //perror(errMsg.c_str());
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
                    std::string errMsg = "❌ Error: No se pudo iniciar la grabación. Archivo: " + audioFile;
                    //std::cerr << errMsg << std::endl;
                    logMsg(errMsg);
                    return;
                } else {
                    std::string msg = "✅ Archivo de audio creado: " + audioFile 
                    + " (Tamaño: " + std::to_string(std::filesystem::file_size(audioFile)) + " bytes)";
                    //std::cout << msg << std::endl;
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
    std::cout << "Press'S' to stop." << std::endl;
    auto start_time = std::chrono::steady_clock::now();
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            printf("\b \b"); // Remove character from terminal
            if (key == 's' || key == 'S') {
                std::cout << "🛑 Stopping..." << std::endl;
                if (system(stopCommand) != 0) {
                    if (errno != 0) {
                    std::string errMsg = "❌ Error al detener la grabación";
                        //perror(errMsg.c_str());
                        logMsg(errMsg);
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                
                //Verify the file exists after stopping the recording.
                if (std::filesystem::exists(audioFile)) {
                    std::string msg = "✅ Archivo de audio encontrado tras detener la grabación: " + audioFile +
                                      " (Tamaño: " + std::to_string(std::filesystem::file_size(audioFile)) + " bytes)";
                    //std::cout << msg << std::endl;
                    logMsg(msg);
                } else {
                    std::string errMsg = "❌ Error: El archivo de audio no se encontró tras detener la grabación.";
                    //std::cerr << errMsg << std::endl;
                    logMsg(errMsg);
                }
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

// Load WAV file and convert it to a normalized float vector
std::vector<float> Transcriber::load_audio(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::string errMsg = "❌ Archivo de audio no existe: " + filename;
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }

    // Read the WAV header (assumed to be 44 bytes)
    char header[44];
    file.read(header, 44);
    if (file.gcount() < 44) {
        std::string errMsg = "❌ Error: Encabezado WAV insuficiente.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }

    // Determine the size of the data section by using the total file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    std::streamoff dataSize = fileSize - static_cast<std::streamoff>(44); // remaining bytes after header
    if (dataSize <= 0) {
        std::string errMsg = "❌ No hay datos de audio.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    if (dataSize % sizeof(int16_t) != 0) {
        std::string errMsg = "❌ Tamaño de datos de audio no alineado a muestras de 16 bits.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    size_t numSamples = dataSize / sizeof(int16_t);

    // Return to the beginning of the data section
    file.seekg(44, std::ios::beg);
    std::vector<int16_t> samples(numSamples);
    file.read(reinterpret_cast<char*>(samples.data()), dataSize);
    if (static_cast<size_t>(file.gcount()) < numSamples * sizeof(int16_t)) {
        std::string errMsg = "❌ Error al leer las muestras de audio.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return {};
    }
    file.close();

    // Normalize samples to floats in range [-1.0, 1.0)
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
        std::string errMsg = "❌ No se pudo cargar el audio.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return "";
    }

    WhisperConfig params = whisper_crear_parametros(WHISPER_SAMPLING_GREEDY);
    params.language = "en";
    params.print_progress = false;

    if (whisper_full(ctx, params, audioData.data(), audioData.size()) != 0) {
        std::string errMsg = "❌ Error al transcribir el audio.";
        //std::cerr << errMsg << std::endl;
        logMsg(errMsg);
        return "";
    }

    int num_segments = whisper_num_segmentos(ctx);
    std::string transcript = "You:";
    for (int i = 0; i < num_segments; ++i) {
        transcript += whisper_obtener_texto_segmento(ctx, i);
        transcript += " ";
    }

    return transcript;
}