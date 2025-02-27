g++ -std=c++17 -fsanitize=undefined OVA2.cpp -I/home/felipeg/whisper.cpp/include -I/home/felipeg/whisper.cpp/ggml/include -L/home/felipeg/whisper.cpp/build/src -lwhisper ../utilities/call_the_model.cpp -o OVA.out -g

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "whisper.h"
#include "../utilities/call_the_model.hpp" 

using AudioFile = const char*;
using Command = const char*;

AudioFile audioFile = "audio.wav";
Command recordCommand = "arecord -f S16_LE -r 16000 -c 1 audio.wav &"; // Linux (Alsa)
Command stopCommand = "pkill -f arecord";  // Stop recording

struct whisper_context *ctx;

// Function prototypes
int kbhit();
void start_microphone();
void stop_microphone();
std::vector<float> load_audio(const std::string &filename);
std::string transcribe_audio();

int main() {
    // Initialize Whisper
    struct whisper_context_params wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params("/home/felipeg/whisper.cpp/models/ggml-base.bin", wparams);
    
    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        return 1;
    }

    // Load chatbot options and history
    std::string historial_json = "historial_test.json";  
    std::string opciones_json = "opcions.json";  

    ollama::options opciones;
    inicializar_opciones(opciones_json, opciones);

    ollama::messages historial;
    inicializar_historial(historial_json, historial);

    std::string modelo = opciones["model"];
    std::string initial_instruction = opciones["initial_intrucion"];

    verificar_ollama(modelo);

    while (true) {
        std::cout << "\n🔴 Presiona 'R' para grabar audio y 'S' para detener. Escribe '/bye' para salir." << std::endl;

        start_microphone();  // Wait for 'R' to start recording
        stop_microphone();   // Wait for 'S' to stop recording

        std::cout << "✅ Grabación finalizada. Transcribiendo...\n";
        std::string prompt = transcribe_audio();

        if (prompt.empty()) {
            std::cerr << "⚠️ No se pudo obtener la transcripción del audio.\n";
            continue;
        }

        std::cout << "📝 Transcripción: " << prompt << std::endl;

        if (prompt == "/bye") {
            std::cout << "Saliendo del chat...\n";
            break;
        }

        // Generate response using chatbot
        obtener_respuesta(historial, modelo, opciones, initial_instruction, prompt, "user");
        std::cout << "🤖 Asistente: " << historial.back()["content"] << "\n";
    }

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
void start_microphone() {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Prevents high CPU usage
    }
}

// Function to stop recording
void stop_microphone() {
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
std::string transcribe_audio() {
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
