#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include "ollama.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sstream>

// Alias para el namespace JSON
using json = nlohmann::json;

// Estructura para almacenar los mensajes del historial
struct Mensaje {
        std::string role;
        std::string content;
    };

    // Función para verificar si Ollama está corriendo e iniciarlo si es necesario

void verificar_ollama(const std::string& modelo) {
        if (!ollama::is_running()) {
            std::cout << "Ollama no está corriendo. Intentando iniciar...\n";
            bool cargado = ollama::load_model(modelo);
            if (cargado) {
                std::cout << "Modelo '" << modelo << "' cargado exitosamente.\n";
            } else {
                std::cerr << "Error: No se pudo iniciar Ollama con el modelo '" << modelo << "'.\n";
                exit(1);
            }
        } else {
            std::cout << "Ollama ya está en ejecución.\n";
        }
    }


    // Función para generar una respuesta y actualizar el historial
void obtener_respuesta(
            ollama::messages& historial, 
            const std::string& modelo, 
            const ollama::options& opciones,
            const std::string& initial_instruction,
            const std::string& prompt,
            const std::string speaking_role
        )
    {
        ollama::messages mensajes = historial;

        // 1) Añadimos al historial el mensaje de tipo "system"
        mensajes.push_back({"system", initial_instruction});

        // 2) Añadimos al historial el mensaje del usuario
        mensajes.push_back({speaking_role, prompt});

        // 4) Consultamos al modelo con la lista completa de mensajes
        std::string respuesta = ollama::chat(modelo, mensajes, opciones);

        // 5) Agregamos la respuesta del modelo al historial
        historial.push_back({speaking_role,prompt});
        historial.push_back({speaking_role, respuesta});

}

        

    // Función para inicializar el historial del chat desde un JSON
void inicializar_historial(const std::string& ruta_json, ollama::messages& historial) {
        std::ifstream archivo(ruta_json);
        if (!archivo) {
            std::cerr << "Error: No se pudo abrir el archivo JSON: " << ruta_json << std::endl;
            return;
        }

        // Leer el archivo JSON
        json json_data;
        archivo >> json_data;
        
        // Convertir a ollama::messages
        for (const auto& item : json_data) {
            if (item.contains("role") && item.contains("content")) {
                historial.push_back({ item["role"].get<std::string>(), item["content"].get<std::string>() });
            }
        }

        std::cout << "Historial cargado desde " << ruta_json << "\n";
}

void inicializar_opciones(const std::string& ruta_json, ollama::options& opciones) {
    std::ifstream archivo(ruta_json);
    if (!archivo) {
        std::cerr << "Error: No se pudo abrir el archivo JSON: " << ruta_json << std::endl;
        return;
    }

    // Leer el archivo JSON
    json json_data;
    archivo >> json_data;

    // Convertir a ollama::options
    for (auto& [clave, valor] : json_data.items()) {
        if (valor.is_number_integer()) {
            opciones[clave] = valor.get<int>();
        } else if (valor.is_number_float()) {
            opciones[clave] = valor.get<double>();
        } else if (valor.is_string()) {
            opciones[clave] = valor.get<std::string>();
        } else {
            std::cerr << "Advertencia: Clave ignorada en el JSON, tipo de dato no compatible -> " << clave << std::endl;
        }
    }

    std::cout << "Opciones cargadas desde " << ruta_json << "\n";
}


void print_formatted_output(const std::string& input) {
    std::istringstream stream(input);
    std::string line;

    std::cout << "================ Asistant out ================\n\n";

    while (std::getline(stream, line)) {
        // Trim leading spaces
        size_t first_char = line.find_first_not_of(" \t");
        if (first_char != std::string::npos) {
            line = line.substr(first_char);
        }

        // Highlight commands in green
        if (line.find("`") != std::string::npos) {
            size_t start = line.find("`");
            size_t end = line.rfind("`");

            if (start != std::string::npos && end != std::string::npos && start != end) {
                std::cout << line.substr(0, start);
                std::cout << "\033[1;32m" << line.substr(start + 1, end - start - 1) << "\033[0m"; // Green text
                std::cout << line.substr(end + 1) << std::endl;
            } else {
                std::cout << line << std::endl;
            }
        } else {
            std::cout << line << std::endl;
        }
    }

    std::cout << "\n==========================================================\n";
}



// Función principal (main)
int main() {
        std::string historial_json = "historial_test.json";  // Archivo JSON con historial previo
        std::string opciones_json = "opcions.json";  // Archivo JSON con historial previo

        ollama::options opciones;
        inicializar_opciones(opciones_json,opciones);

        ollama::messages historial = {};  // Historial del chat
        inicializar_historial(historial_json, historial);

        std::string modelo = opciones["model"];//"deepseek-coder";  // Nombre del modelo a usar

        std::string initial_instruction = opciones["initial_intrucion"];

        // Verificar e iniciar Ollama
        verificar_ollama(modelo);

        // Bucle de tres interacciones
        for (int i = 0; i < 3; i++) {
            std::string prompt;
            std::cout << "Tú: ";
            std::getline(std::cin, prompt);
            
            obtener_respuesta(historial,modelo,opciones,initial_instruction,prompt,"user");
            print_formatted_output(historial.back()["content"]);

        }

        return 0;
}