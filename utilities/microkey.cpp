//g++ microkey.cpp -I/home/felipeg/whisper.cpp/include -I/home/felipeg/whisper.cpp/ggml/include -L/home/felipeg/whisper.cpp/build/src -lwhisper -o microkey
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "whisper.h"
#include <string>   

using AudioFile = const std::string;
using Command = const std::string;


struct whisper_context *ctx;

// Function prototypes
int kbhit();
void start_microphone(AudioFile &filename);
void stop_microphone();
std::vector<float> load_audio(const std::string &filename);
void transcribe_audio(const std::string &filename);

int main() {
    // Initialize Whisper with the correct parameters

    AudioFile audioFile = "audios/audio.wav";
    struct whisper_context_params wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params("../whisper.cpp/models/ggml-base.bin", wparams);
    
    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        return 1;
    }

    start_microphone(audioFile);  // Wait for 'R' to start recording
    stop_microphone();   // Wait for 'S' to stop recording
    std::cout << "✅ Grabación finalizada. Archivo guardado en " << audioFile << std::endl;

    transcribe_audio(audioFile); // Transcribe recorded audio

    whisper_free(ctx); // Free Whisper memory
    return 0;
}

// Function to detect key presses (non-blocking)
int kbhit() {
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
void start_microphone(AudioFile &filename) {

    Command recordCommand = "arecord -f S16_LE -r 16000 -c 1 " + filename + "&"; // Linux (Alsa)

    std::cout << "🎤 Presiona 'R' para comenzar la grabación..." << std::endl;
    while (true) {
        if (kbhit()) {
            char key = getchar();
            if (key == 'r' || key == 'R') {
                std::cout << "🎙️ Grabando... Presiona 'S' para detener." << std::endl;
                if (system(recordCommand.c_str()) != 0) {
                    std::cerr << "❌ Error al iniciar la grabación." << std::endl;
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Prevents high CPU usage
    }
}

// Function to stop recording
void stop_microphone() {
    Command checkCommand = "pgrep -f arecord > /dev/null";  // Check if process exists
    Command stopCommand = "pkill -f arecord";  // Stop recording    

    std::cout << "Presiona 'S' para detener la grabación..." << std::endl;
    while (true) {
        if (kbhit()) {
            char key = getchar();
            if (key == 's' || key == 'S') {
                std::cout << "🛑 Deteniendo grabación..." << std::endl;

                // Check if the process exists before killing it
                int isRunning = system(checkCommand.c_str());
                if (isRunning != 0) {
                    std::cerr << "⚠️  No se encontró el proceso de grabación (arecord no está corriendo).\n";
                    return;
                }

                // Stop recording
                int state = system(stopCommand.c_str());
                
                if (state != 0) {
                    std::cerr << "❌ Error al detener la grabación.\n";
                } else {
                    std::cout << "✅ Grabación detenida correctamente.\n";
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Prevents high CPU usage
    }
}

// Function to load the WAV audio file into a buffer
std::vector<float> load_audio(const std::string &filename) {
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
void transcribe_audio(const std::string &filename) {
    std::vector<float> audioData = load_audio(filename);
    if (audioData.empty()) {
        std::cerr << "❌ No se pudo cargar el audio." << std::endl;
        return;
    }

    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.language = "es";  // Set language explicitly
    params.print_progress = false;  // Optional: Disable progress output

    if (whisper_full(ctx, params, audioData.data(), audioData.size()) != 0) {
        std::cerr << "❌ Error al transcribir el audio." << std::endl;
        return;
    }

    int num_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < num_segments; ++i) {
        const char *text = whisper_full_get_segment_text(ctx, i);
        std::cout << "📝 Transcripción: " << text << std::endl;
    }
}
