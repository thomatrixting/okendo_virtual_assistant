// Compile with g++ -std=c++17 -fsanitize=undefined speak_with_the_model.cpp ../utilities/call_the_model.cpp -o chat.out -g
#include <iostream>
#include "../utilities/call_the_model.hpp"  // Incluir el header

// Function to display help information
void show_help();

int main(int argc, char* argv[]) {

    // Check for help or detailed flag
    bool detailed_response = false;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0) {
            show_help();
            return 0;
        } else if (std::strcmp(argv[i], "-d") == 0) {
            detailed_response = true;
        } else {
            std::cerr << "Invalid argument: " << argv[i] << "\n";
            show_help();
            return 1;
        }
    }
    std::string comand_dir = get_commands_directory();
    
    std::string historial_json = comand_dir+"/historial_test.json";  
    std::string options_json = comand_dir+"/options.json";  

    ollama::options options;
    initialize_options(options_json, options);

    ollama::messages historial;
    initialize_history(historial_json, historial);

    std::string model = options["model_chat_response"];
    std::string initial_instruction = options["initial_instruction"];

    // Adjust for detailed response if flag is set
    if (detailed_response) {
        initial_instruction = options["detail_initial_instruction"];
        std::string model = options["model_chat_response_unrestricted"];

    }

    verify_ollama(model);

    while (true) {
        std::string prompt;
        std::cout << "You: ";
        std::getline(std::cin, prompt);

        if (prompt == "bye") {
            std::cout << "Leaving the chat...\n";
            break;
        }

        get_response(historial, model, options, initial_instruction, prompt, "user");
        print_formatted_output(historial.back()["content"]);
    }


    return 0;
}

inline void show_help() {
    std::cout << "Usage: ./session_chat [-d]\n"
              << "  -d        Start the session with detailed responses.\n"
              << "  --help    Show this help message.\n";
}

