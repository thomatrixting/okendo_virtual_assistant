//compile with g++ -std=c++17 -fsanitize=undefined speak_with_the_model.cpp ../utilities/call_the_model.cpp -o a.out -g
#include <iostream>
#include "../utilities/call_the_model.hpp"  // Incluir el header

int main() {
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
        std::string prompt;
        std::cout << "Tú: ";
        std::getline(std::cin, prompt);

        if (prompt == "/bye") {
            std::cout << "Saliendo del chat...\n";
            break;
        }

        obtener_respuesta(historial, modelo, opciones, initial_instruction, prompt, "user");
        print_formatted_output(historial.back()["content"]);
    }


    return 0;
}
