//g++ -std=c++17 -fsanitize=undefined OVA10.cpp -I/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/include -I/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/ggml/include -L/home/felipeg/okendo_virtual_assistant/utilities/whisper.cpp/build/src -lwhisper ../utilities/json_parser.cpp ../utilities/call_the_model.cpp ../utilities/transcriber.cpp ../examples/amfq.cpp ../examples/chat.cpp ../utilities/voicer.cpp -o OVA10.out -g
#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include "../utilities/call_the_model.hpp"
#include "../utilities/transcriber.hpp"
#include "../utilities/voicer.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

// Funciones auxiliares
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput);
std::unordered_map<std::string, std::string> loadOptions(const std::string& filename);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chat|amfq] [--voice] [--speak]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    bool useVoiceInput = false, useVoiceOutput = false;
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--voice") useVoiceInput = true;
        else if (arg == "--speak") useVoiceOutput = true;
    }
    
    if (mode == "chat" || mode == "amfq") {
        runMode(mode, useVoiceInput, useVoiceOutput);
    } else {
        std::cerr << "Invalid mode. Please use 'chat' or 'amfq'." << std::endl;
        return 1;
    }
    
    return 0;
}

std::unordered_map<std::string, std::string> loadOptions(const std::string& filename) {
    std::unordered_map<std::string, std::string> options;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return options;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            options[key] = value;
        }
    }
    return options;
}

std::string getResponse(const std::string& query) {
    std::string command_dir = get_commands_directory();
    std::string historial_json = command_dir + "/historial_test.json";
    std::string opciones_json = command_dir + "/opcions.json";
    
    auto opciones = loadOptions(opciones_json);
    if (opciones.find("model") == opciones.end() || opciones.find("initial_intrucion") == opciones.end()) {
        return "Error: Could not initialize options.";
    }
    
    ollama::messages historial;
    if (!inicializar_historial(historial_json, historial)) {
        historial.clear();
    }
    
    try {
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
    Voicer("transcripcion.txt", "audiogene.wav").generarAudio(text);
}

void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput) {
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
        
        std::string response = getResponse(input);
        std::cout << "Assistant: " << response << std::endl;
        
        if (useVoiceOutput) std::thread(speak, response).detach();
        
        if (mode == "amfq") break;
    }
}
