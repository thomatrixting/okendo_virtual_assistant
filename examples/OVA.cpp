//g++ -std=c++17 -fsanitize=undefined OVA.cpp -I ../utilities/whisper.cpp/include -I ../utilities/whisper.cpp/ggml/include -L ../utilities/whisper.cpp/build/src -lwhisper ../utilities/call_the_model.cpp ../utilities/transcriber.cpp ../utilities/voicer.cpp -o OVA.out -g

#include <cctype>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "../utilities/voicer.hpp"
#include "../utilities/transcriber.hpp"
#include "../utilities/call_the_model.hpp"

// Functions
void speak(const std::string& text);
std::string getResponse(const std::string& query,const std::string& mode,bool &detail_response);
void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput, bool detail_response);
std::unordered_map<std::string, std::string> loadOptions(const std::string& filename);
void OVAlog(const std::string& message);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << "ova" << " [chat|amfq] [--voice] [--speak]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    bool useVoiceInput = false, useVoiceOutput = false; bool Detail_response = false;
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--voice") useVoiceInput = true;
        if (arg == "--speak") useVoiceOutput = true;
        if (arg == "--detail") Detail_response = true;
    }
    
    if (mode == "chat" || mode == "amfq") {
        runMode(mode, useVoiceInput, useVoiceOutput, Detail_response);
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
        std::cerr << " ❌ Error: Failed to open log file in " << logFilePath << std::endl;
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

std::string getResponse(const std::string& query,const std::string& mode,bool &detail_response) {
    std::string command_dir = get_commands_directory();
    std::string historial_json = command_dir + "/historial_test.json";
    std::string options_json = command_dir + "/options.json";
    
    auto options = loadOptions(options_json);
    
    // Ensure required keys exist
    std::vector<std::string> required_keys = {"model", "initial_instruction"};
    for (const auto& key : required_keys) {
        if (options.find(key) == options.end()) {
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
        std::string model = options["model"];

        if (!detail_response){
            if(mode == "amfq"){
                model = options["model_fast_response"];
            } else {
                model = options["model_chat_response"];

            }
        } else {
            if(mode == "amfq"){
                model = options["model_chat_response"];
            } else {
                model = options["model_chat_response_unrestricted"];
            }
        }

        verificar_ollama(model);
        
        // Convert unordered_map to class ollama::options
        ollama::options convertedOptions;
        convertedOptions["model"] = model;

        if (detail_response){
            convertedOptions["initial_intrucion"] = options["detail_initial_intrucion"];
        } else {
            convertedOptions["initial_intrucion"] = options["initial_intrucion"];
        }
        
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
    Voicer("transcription.txt", "audiogene.wav").generarAudio(text);
}

void runMode(const std::string& mode, bool useVoiceInput, bool useVoiceOutput, bool detail_response) {
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

        if (input.find("bye") != std::string::npos) { 
            system("pkill aplay");
            break;
        }

        std::string response = getResponse(input, mode, detail_response);

        // Handle voice output correctly
        if (useVoiceOutput) {
            if (mode == "amfq") {
                // If in AMFQ mode, wait for speak to finish
                std::thread speechThread(speak, response);
                speechThread.join();
            } else {
                // If in chat mode, run speak in the background
                std::thread(speak, response).detach();
            }
        }

        if (mode == "amfq") break;
    }

    system("pkill aplay"); // Ensure aplay is stopped at the very end
}