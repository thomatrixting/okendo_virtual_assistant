#include <iostream>
#include <string>
#include <thread>
#include <cstdlib>
#include "../utilities/call_the_model.hpp"

// Forward declarations
void speak(const std::string& text);
std::string getResponse(const std::string& query);
void chatMode(bool useVoice = false);
void amfqMode(bool useVoice = false);

int main(int argc, char* argv[]) {
    // Ensure a mode is provided.
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [chat|amfq] [--voice]" << std::endl;
        return 1;
    }
    
    std::string mode = argv[1];
    bool useVoice = false;
    
    // Check for the optional voice flag.
    if (argc > 2 && std::string(argv[2]) == "--voice") {
        useVoice = true;
    }
    
    if (mode == "chat") {
        chatMode(useVoice);
    } else if (mode == "amfq") {
        amfqMode(useVoice);
    } else {
        std::cerr << "Invalid mode. Please use 'chat' or 'amfq'." << std::endl;
        return 1;
    }
    
    return 0;
}

// Process the query and return a response.
// The function now simply processes the given query instead of running its own loop.
std::string getResponse(const std::string& query) {
    // Setup directories and file paths
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
    
    // Check for required keys (corrected key names)
    if (opciones.find("model") == opciones.end() ||
        opciones.find("initial_intrucion") == opciones.end()) {
        std::cerr << "Required options not found in " << opciones_json << std::endl;
        return "Error: Missing required options.";
    }
    
    ollama::messages historial;
    try {
        inicializar_historial(historial_json, historial);
    } catch (const std::exception& e) {
        std::cerr << "Error initializing historial: " << e.what() << std::endl;
        // Optionally, you could continue with an empty historial if that is acceptable.
    }
    
    std::string modelo = opciones["model"];
    std::string initial_instruction = opciones["initial_intrucion"];
    
    // Optionally set detailed response if desired.
    bool detailed_response = false;
    if (detailed_response && opciones.find("detail_initial_intrucion") != opciones.end()) {
        initial_instruction = opciones["detail_initial_intrucion"];
    }
    
    // Verify the model (error handling via try-catch)
    try {
        verificar_ollama(modelo);
    } catch (const std::exception& e) {
        std::cerr << "Error verifying model: " << e.what() << std::endl;
        return "Error: Model verification failed.";
    }
    
    // Process the provided query.
    try {
        obtener_respuesta(historial, modelo, opciones, initial_instruction, query, "user");
    } catch (const std::exception& e) {
        std::cerr << "Error obtaining response: " << e.what() << std::endl;
        return "Error: Could not obtain response.";
    }
    
    // Check if historial has at least one message with a "content" field.
    if (historial.empty() || historial.back().find("content") == historial.back().end()) {
        std::cerr << "No response available in historial." << std::endl;
        return "Error: No response received.";
    }
    
    std::string response = historial.back()["content"];
    print_formatted_output(response);
    return response;
}

// Text-to-speech function for Linux using a 'espeak' command.
// Note: This assumes that a 'speak' command exists on the system.
void speak(const std::string& text) {
    std::string command = "espeak \"" + text + "\"";
    int ret = std::system(command.c_str());
    if (ret != 0) {
        std::cerr << "Error: speak command failed with code " << ret << std::endl;
    }
}

// Chat mode: interactive loop that reads user input, obtains a response, and optionally uses voice.
void chatMode(bool useVoice) {
    std::cout << "Entering Chat Mode. Type 'exit' to quit." << std::endl;
    std::string input;
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, input);
        if (input == "exit") {
            std::cout << "Exiting Chat Mode..." << std::endl;
            break;
        }
        std::string response = getResponse(input);
        std::cout << "Assistant: " << response << std::endl;
        if (useVoice) {
            // Run speak in a detached thread to avoid blocking input.
            std::thread t(speak, response);
            t.detach();
        }
    }
}

// AMFQ mode: single question/answer interaction.
void amfqMode(bool useVoice) {
    std::cout << "Entering AMFQ Mode. Ask your question:" << std::endl;
    std::string question;
    std::getline(std::cin, question);
    std::string response = getResponse(question);
    std::cout << "Assistant: " << response << std::endl;
    if (useVoice) {
        speak(response);
    }
}
