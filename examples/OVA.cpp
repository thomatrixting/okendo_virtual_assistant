//g++ -std=c++17 -fsanitize=undefined OVA.cpp -I ../utilities/whisper.cpp/include -I ../utilities/whisper.cpp/ggml/include -L ../utilities/whisper.cpp/build/src -lwhisper ../utilities/call_the_model.cpp ../utilities/transcriber.cpp ../utilities/voicer.cpp -o OVA.out -g

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
#include <cctype>

// Funciones auxiliares
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput);
std::unordered_map<std::string, std::string> loadOptions(const std::string& filename);
void OVAlog(const std::string& message);

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

void OVAlog(const std::string& message) {
    
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "OVA.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "❌ Error: No se pudo abrir el archivo de registro en " << logFilePath << std::endl;
    }
}

std::unordered_map<std::string, std::string> loadOptions(const std::string& filename) {
    std::unordered_map<std::string, std::string> options;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::string errMsg = "Error: Could not open " + filename;
        OVAlog(errMsg);
        return options;
    }

    try {
        nlohmann::json jsonData;
        file >> jsonData;
        
        for (auto& [key, value] : jsonData.items()) {
            options[key] = value.is_string() ? value.get<std::string>() : value.dump();
        }

        std::string Msg = "Loaded options:\n";
        OVAlog(Msg);
        for (const auto& [key, value] : options) {
            std::string Msg = key + " -> " + value;
            OVAlog(Msg);
        }
        
    } catch (const std::exception& e) {
        std::string errMsg = std::string("JSON parsing error: ") + e.what();
        OVAlog(errMsg);
    }

    return options;
}

std::string getResponse(const std::string& query) {
    std::string command_dir = get_commands_directory();
    std::string historial_json = command_dir + "/historial_test.json";
    std::string opciones_json = command_dir + "/opcions.json";
    
    auto opciones = loadOptions(opciones_json);
    
    // Ensure required keys exist
    std::vector<std::string> required_keys = {"model", "initial_intrucion"};
    for (const auto& key : required_keys) {
        if (opciones.find(key) == opciones.end()) {
            std::string errMsg = "Error: Missing required key in options: " + key;
            OVAlog(errMsg);
        }
    }
    
    ollama::messages historial;
    inicializar_historial(historial_json, historial);
    if (historial.empty()) {
        historial.clear();
    }
    
    try {
        verificar_ollama(opciones["model"]);
        
        // Convert unordered_map to ollama::options class
        ollama::options convertedOptions;
        convertedOptions["model"] = opciones["model"];
        convertedOptions["initial_intrucion"] = opciones["initial_intrucion"];
        
        obtener_respuesta(historial, convertedOptions["model"], convertedOptions, convertedOptions["initial_intrucion"], query, "user");
        if (!historial.empty() && historial.back().count("content")) {
            print_formatted_output(historial.back()["content"]);
            return historial.back()["content"];
        }
    } catch (const std::exception& e) {
        //left logging
        std::string errMsg = std::string("Exception caught: ") + e.what();
        OVAlog(errMsg);
    } catch (...) {
        //left logging
        std::string errMsg = "Unknown error occurred.";
        OVAlog(errMsg);
    }
    //left logging
    return "Error: No response received.";
}

void speak(const std::string& text) {
    Voicer("transcripcion.txt", "audiogene.wav").generarAudio(text);
}

void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput) {
    Transcriber transcriber("../utilities/whisper.cpp/models/ggml-base.bin", "audio.wav");
    std::cout << "Entering " << (mode == "chat" ? "Chat" : "AMFQ") << " Mode. Say or type 'exit' to quit." << std::endl;

    std::string input;
    while (true) {
        std::cout << "You: ";
        
        if (useVoiceInput) {
            transcriber.start_microphone();
            system("pkill aplay");
            transcriber.stop_microphone();
            input = transcriber.transcribe_audio();
            
            if (input.empty()) {
                std::cerr << "Error: Voice input failed." << std::endl;
                continue;
            }

            // Trim spaces, newlines, '.', and '!' from the input
            input.erase(0, input.find_first_not_of(" \t\r\n.!")); // Trim left
            input.erase(input.find_last_not_of(" \t\r\n.!") + 1); // Trim right
            std::transform(input.begin(), input.end(), input.begin(), ::tolower);
            
            std::cout << input << std::endl;
        } else {
            std::getline(std::cin, input);
        }

        if (input == "you: exit") { 
        system("pkill aplay"); 
        break;
        }

        std::string response = getResponse(input);
        if (useVoiceOutput) std::thread(speak, response).detach();

        if (mode == "amfq") break;
    }
}