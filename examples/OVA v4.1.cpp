#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <algorithm>
#include "../utilities/call_the_model.hpp"
#include "../utilities/transcriber.hpp"
#include "../utilities/voicer.hpp"

// Funciones auxiliares
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput);

int main(int argc, char* argv[]) {
    // Verifica que al menos se proporcione un argumento para el modo de ejecución
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chat|amfq] [--voice] [--speak]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower); // Convierte el modo a minúsculas
    bool useVoiceInput = false, useVoiceOutput = false;
    
    // Procesa los argumentos adicionales
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--voice") useVoiceInput = true;
        else if (arg == "--speak") useVoiceOutput = true;
    }
    
    // Verifica el modo y ejecuta la función correspondiente
    if (mode == "chat" || mode == "amfq") {
        runMode(mode, useVoiceInput, useVoiceOutput);
    } else {
        std::cerr << "Invalid mode. Please use 'chat' or 'amfq'." << std::endl;
        return 1;
    }
    
    return 0;
}

std::string getResponse(const std::string& query) {
    std::string command_dir = get_commands_directory();
    std::string historial_json = command_dir + "/historial_test.json";
    std::string opciones_json = command_dir + "/opcions.json";
    
    ollama::options opciones;
    // Inicializa las opciones desde el archivo JSON
    if (!inicializar_opciones(opciones_json, opciones) || opciones.count("model") == 0 || opciones.count("initial_intrucion") == 0)
        return "Error: Could not initialize options.";
    
    ollama::messages historial;
    // Intenta inicializar el historial, si falla, lo deja vacío
    if (!inicializar_historial(historial_json, historial))
        historial.clear();
    
    try {
        // Verifica el modelo y obtiene una respuesta
        verificar_ollama(opciones["model"]);
        obtener_respuesta(historial, opciones["model"], opciones, opciones["initial_intrucion"], query, "user");
        if (!historial.empty() && historial.back().count("content")) {
            print_formatted_output(historial.back()["content"]);
            return historial.back()["content"];
        }
    } catch (...) {}
    
    return "Error: No response received.";
}

void speak(const std::string& text) {
    // Genera un archivo de audio a partir del texto
    Voicer("transcripcion.txt", "audiogene.wav").generarAudio(text);
}

void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput) {
    // Inicializa el transcriptor para manejar entrada de voz si es necesario
    Transcriber transcriber("../utilities/whisper.cpp/models/ggml-base.bin", "audio.wav");
    std::cout << "Entering " << (mode == "chat" ? "Chat" : "AMFQ") << " Mode. Type 'exit' to quit." << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "You: ";
        if (useVoiceInput) {
            // Captura entrada de voz y la transcribe
            transcriber.start_microphone();
            transcriber.stop_microphone();
            input = transcriber.transcribe_audio();
            std::cout << input << std::endl;
        } else {
            std::getline(std::cin, input);
        }
        if (mode == "chat" && input == "exit") break; // Permite salir en modo chat
        
        // Obtiene la respuesta del modelo
        std::string response = getResponse(input);
        std::cout << "Assistant: " << response << std::endl;
        
        // Si la salida de voz está activada, genera un hilo para hablar la respuesta
        if (useVoiceOutput) std::thread(speak, response).detach();
        
        if (mode == "amfq") break; // En modo AMFQ, se ejecuta solo una vez
    }
}
