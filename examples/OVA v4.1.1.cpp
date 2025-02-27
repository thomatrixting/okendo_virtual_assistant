//g++ -std=c++17 -fsanitize=undefined OVA9.cpp -I/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/include -I/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/ggml/include -L/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/build/src -lwhisper ../utilities/json_parser.cpp ../utilities/call_the_model.cpp ../utilities/transcriber.cpp ../examples/amfq.cpp ../examples/chat.cpp ../utilities/voicer.cpp -o OVA9.out -g
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include "../utilities/call_the_model.hpp"
#include "../utilities/transcriber.hpp"
#include "../utilities/voicer.hpp"
#include "../utilities/json_parser.hpp"

// Funciones auxiliares
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput);

int main(int argc, char* argv[]) {
    // Verifica que se pase al menos un argumento
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chat|amfq] [--voice] [--speak]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    bool useVoiceInput = false, useVoiceOutput = false;
    
    // Procesa argumentos adicionales
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--voice") useVoiceInput = true;
        else if (arg == "--speak") useVoiceOutput = true;
    }
    
    // Verifica si el modo es válido y ejecuta la función correspondiente
    if (mode == "chat" || mode == "amfq") {
        runMode(mode, useVoiceInput, useVoiceOutput);
    } else {
        std::cerr << "Invalid mode. Please use 'chat' or 'amfq'." << std::endl;
        return 1;
    }
    
    return 0;
}

std::string getResponse(const std::string& query) {
    // Carga rutas de archivos de historial y opciones
    std::string command_dir = get_commands_directory();
    std::string historial_json = command_dir + "/historial_test.json";
    std::string opciones_json = command_dir + "/opcions.json";
    
    ollama::options opciones;
    inicializar_opciones(opciones_json, opciones);
    if (opciones.find("model") == opciones.end() || opciones.find("initial_intrucion") == opciones.end()) {
        return "Error: Could not initialize options.";
    }
    
    ollama::messages historial;
    inicializar_historial(historial_json, historial);
    
    try {
        // Verifica el modelo y obtiene la respuesta
        verificar_ollama(opciones["model"]);
        obtener_respuesta(historial, opciones["model"], opciones, opciones["initial_intrucion"], query, "user");
        if (!historial.empty() && historial.back().count("content")) {
            print_formatted_output(historial.back()["content"]);
            return historial.back()["content"];
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
    }
    
    return "Error: No response received.";
}

void speak(const std::string& text) {
    // Genera audio a partir del texto
    Voicer("transcripcion.txt", "audiogene.wav").generarAudio(text);
}

void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput) {
    // Inicializa el transcriptor si se usa entrada de voz
    Transcriber transcriber("../utilities/whisper.cpp/models/ggml-base.bin", "audio.wav");
    std::cout << "Entering " << (mode == "chat" ? "Chat" : "AMFQ") << " Mode. Type 'exit' to quit." << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "You: ";
        if (useVoiceInput) {
            transcriber.start_microphone();
            transcriber.stop_microphone();
            input = transcriber.transcribe_audio();
            if (input.empty()) {
                std::cerr << "Error: Voice input failed." << std::endl;
                continue;
            }
            std::cout << input << std::endl;
        } else {
            std::getline(std::cin, input);
        }
        if (mode == "chat" && input == "exit") break;
        
        // Obtiene y muestra la respuesta del asistente
        std::string response = getResponse(input);
        std::cout << "Assistant: " << response << std::endl;
        
        // Genera salida de voz si está activada
        if (useVoiceOutput) std::thread(speak, response).detach();
        
        if (mode == "amfq") break;
    }
}
