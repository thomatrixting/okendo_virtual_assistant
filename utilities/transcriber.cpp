#include "transcriber.hpp"
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

// Constructor: Initializes Whisper
Transcriber::Transcriber(const std::string &modelPath, const std::string &audioPath) {
    struct whisper_context_params wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params(modelPath.c_str(), wparams);

    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        exit(1);
    }

    audioFile = audioPath.c_str();
    recordCommand = "arecord -f S16_LE -r 16000 -c 1 audio.wav &"; // Linux (Alsa)
    stopCommand = "pkill -f arecord";
}

// Destructor: Frees Whisper memory
Transcriber::~Transcriber() {
    whisper_free(ctx);
}

// Function to detect key presses (non-blocking)
int Transcriber::kbhit() {
    struct termios oldt, newt;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    int ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

// Function to start recording
void Transcriber::start_microphone() {
    std::cout << "🎤 Presiona 'R' para comenzar la grabación..." << std::endl;
    while (true) {
        if (kbhit()) {
            char key = getchar();
            if (key == 'r' || key == 'R') {
                std::cout << "🎙️ Grabando... Presiona 'S' para detener." << std::endl;
                if (system(recordCommand) != 0) {
                    std::cerr << "❌ Error al iniciar la grabación." << std::endl;
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Function to stop recording
void Transcriber::stop_microphone() {
    std::cout << "Presiona 'S' para detener la grabación..." << std::endl;
    while (true) {
        if (kbhit()) {
            char key = getchar();
            if (key == 's' || key == 'S') {
                std::cout << "🛑 Deteniendo grabación..." << std::endl;
                if (system(stopCommand) != 0) {
                    std::cerr << "❌ Error al detener la grabación." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Function to load WAV audio into a buffer
std::vector<float> Transcriber::load_audio(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "❌ Error al abrir el archivo de audio." << std::endl;
        return {};
    }

    file.seekg(44); // Skip WAV header
    std::vector<int16_t> samples;
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        samples.push_back(sample);
    }
    file.close();

    std::vector<float> audioData(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        audioData[i] = samples[i] / 32768.0f; // Normalize to float
    }

    return audioData;
}

// Function to transcribe the audio using Whisper
std::string Transcriber::transcribe_audio() {
    std::vector<float> audioData = load_audio(audioFile);
    if (audioData.empty()) {
        std::cerr << "❌ No se pudo cargar el audio." << std::endl;
        return "";
    }

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.language = "es";  // Set language explicitly
    params.print_progress = false;  // Optional: Disable progress output

    if (whisper_full(ctx, params, audioData.data(), audioData.size()) != 0) {
        std::cerr << "❌ Error al transcribir el audio." << std::endl;
        return "";
    }

    int num_segments = whisper_full_n_segments(ctx);
    std::string transcript = "";
    for (int i = 0; i < num_segments; ++i) {
        transcript += whisper_full_get_segment_text(ctx, i);
        transcript += " ";
    }

    return transcript;
}
