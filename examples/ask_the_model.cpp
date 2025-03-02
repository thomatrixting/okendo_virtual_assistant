//copile with g++ -std=c++17 -fsanitize=undefined ask_the_model.cpp ../utilities/call_the_model.cpp -o amfq.out -g
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include "../utilities/call_the_model.hpp"  // Include your existing model call functions

// Function to display help information
void show_help();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No prompt provided. Use --help for usage information.\n";
        return 1;
    }

    // Check for help flag
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            show_help();
            return 0;
        }
    }

    // Extract prompt and flags
    std::string prompt;
    bool detailed_response = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-d") == 0) {
            detailed_response = true;
        } else if (argv[i][0] != '-') { // Ignore other flags for now
            prompt += std::string(argv[i]) + " ";
        }
    }

    if (prompt.empty()) {
        std::cerr << "Error: No valid prompt detected. Use --help for usage information.\n";
        return 1;
    }

    std::string comand_dir = get_commands_directory();
    
    std::string historial_json = comand_dir+"/historial_test.json";  
    std::string opciones_json = comand_dir+"/opcions.json";  

    ollama::options opciones;
    inicializar_opciones(opciones_json, opciones);

    ollama::messages historial;
    inicializar_historial(historial_json, historial);

    std::string modelo = opciones["model_fast_response"];
    std::string initial_instruction = opciones["initial_intrucion"];

    // Adjust for detailed response if flag is set
    if (detailed_response) {
        initial_instruction = opciones["detail_initial_intrucion"];
        modelo = opciones["model_chat_response"];
    }

    // Verify if Ollama server is running
    verificar_ollama(modelo);

    // Process the prompt and generate a response
    obtener_respuesta(historial, modelo, opciones, initial_instruction, prompt, "user");
    print_formatted_output(historial.back()["content"]);

    return 0;
}

inline void show_help() {
    std::cout << "Usage: ./amfq [PROMPT] [-d] [--help]\n"
              << "  PROMPT    The question you want to ask the model.\n"
              << "  -d        Request a detailed response.\n"
              << "  --help    Show this help message.\n";
}
