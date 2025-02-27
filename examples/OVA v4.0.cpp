#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include <algorithm>
#include "../utilities/call_the_model.hpp"
#include "../utilities/transcriber.hpp"

// Forward declarations
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void chatMode(bool useVoiceInput = false, bool useVoiceOutput = false);
void amfqMode(bool useVoiceInput = false, bool useVoiceOutput = false);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chat|amfq] [--voice] [--speak]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    bool useVoiceInput = false;
    bool useVoiceOutput = false;
    
    std::transform(mode.begin(), mode.end(), mode.begin(), ::tolower);
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--voice") {
            useVoiceInput = true;
        } else if (arg == "--speak") {
            useVoiceOutput = true;
        }
    }
    
    if (mode == "chat") {
        chatMode(useVoiceInput, useVoiceOutput);
    } else if (mode == "amfq") {
        amfqMode(useVoiceInput, useVoiceOutput);
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
    try {
        inicializar_opciones(opciones_json, opciones);
    } catch (const std::exception& e) {
        std::cerr << "Error initializing options: " << e.what() << std::endl;
        return "Error: Could not initialize options.";
    }
    
    if (opciones.find("model") == opciones.end() ||
        opciones.find("initial_intrucion") == opciones.end()) {
        return "Error: Missing required options.";
    }
    
    ollama::messages historial;
    try {
        inicializar_historial(historial_json, historial);
    } catch (const std::exception& e) {
        historial.clear();
    }
    
    std::string modelo = opciones["model"];
    std::string initial_instruction = opciones["initial_intrucion"];
    
    try {
        verificar_ollama(modelo);
    } catch (const std::exception& e) {
        return "Error: Model verification failed.";
    }
    
    try {
        obtener_respuesta(historial, modelo, opciones, initial_instruction, query, "user");
    } catch (const std::exception& e) {
        return "Error: Could not obtain response.";
    }
    
    if (historial.empty() || historial.back().find("content") == historial.back().end()) {
        return "Error: No response received.";
    }
    
    std::string response = historial.back()["content"];
    print_formatted_output(response);
    return response;
}

void speak(const std::string& text) {
    std::string command = "espeak \"" + text + "\"";
    if (std::system(command.c_str()) != 0) {
        std::cerr << "Error: speak command failed." << std::endl;
    }
}

void chatMode(bool useVoiceInput, bool useVoiceOutput) {
    Transcriber transcriber("../utilities/whisper.cpp/models/ggml-base.bin", "audio.wav");
    std::cout << "Entering Chat Mode. Type 'exit' to quit." << std::endl;
    std::string input;
    while (true) {
        std::cout << "You: ";
        if (useVoiceInput) {
            transcriber.start_microphone();
            transcriber.stop_microphone();
            input = transcriber.transcribe_audio();
            std::cout << input << std::endl;
        } else {
            std::getline(std::cin, input);
        }
        if (input == "exit") break;
        std::string response = getResponse(input);
        std::cout << "Assistant: " << response << std::endl;
        if (useVoiceOutput) {
            std::thread t(speak, response);
            t.detach();
        }
    }
}

void amfqMode(bool useVoiceInput, bool useVoiceOutput) {
    Transcriber transcriber("../utilities/whisper.cpp/models/ggml-base.bin", "audio.wav");
    std::cout << "Entering AMFQ Mode. Ask your question:" << std::endl;
    std::string question;
    if (useVoiceInput) {
        transcriber.start_microphone();
        transcriber.stop_microphone();
        question = transcriber.transcribe_audio();
        std::cout << question << std::endl;
    } else {
        std::getline(std::cin, question);
    }
    std::string response = getResponse(question);
    std::cout << "Assistant: " << response << std::endl;
    if (useVoiceOutput) speak(response);
}
