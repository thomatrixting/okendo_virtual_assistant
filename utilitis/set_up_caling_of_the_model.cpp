#include <iostream>
#include <fstream>
#include <vector>
#include "json.hpp"
#include "ollama.hpp"

// Alias para el namespace JSON
using json = nlohmann::json;

// Estructura para almacenar los mensajes del historial
struct Mensaje {
    std::string role;
    std::string content;
};

// Función para verificar si Ollama está corriendo e iniciarlo si es necesario
void verificar_e_iniciar_ollama(const std::string& modelo) {
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
void generar_respuesta(
    const std::string& prompt, 
    std::vector<Mensaje>& historial, 
    const std::string& modelo, 
    const ollama::options& opciones
) {
    // Agregar el mensaje del usuario al historial
    historial.push_back({"user", prompt});

    // Convertir historial a formato Ollama
    ollama::messages mensajes;
    for (const auto& msg : historial) {
        mensajes.push_back({msg.role, msg.content});
    }

    // Generar respuesta con el modelo
    std::string respuesta = ollama::chat(modelo, mensajes, opciones);

    // Guardar la respuesta en el historial
    historial.push_back({"assistant", respuesta});

    // Mostrar la respuesta
    std::cout << "Asistente: " << respuesta << "\n";
}

// Función para inicializar el historial del chat desde un JSON
void inicializar_historial(const std::string& ruta_json, std::vector<Mensaje>& historial) {
    std::ifstream archivo(ruta_json);
    if (!archivo) {
        std::cerr << "Error: No se pudo abrir el archivo JSON.\n";
        return;
    }

    // Leer el archivo JSON
    json json_data;
    archivo >> json_data;
    
    // Convertir a la estructura del historial
    for (const auto& item : json_data) {
        historial.push_back({item["role"], item["content"]});
    }

    std::cout << "Historial cargado desde " << ruta_json << "\n";
}

// Función principal (main)
int main() {
    std::string modelo = "deepseek-r1:1.5b";  // Nombre del modelo a usar
    std::string archivo_json = "historial_test.json";  // Archivo JSON con historial previo
    std::vector<Mensaje> historial;  // Historial del chat

    // Verificar e iniciar Ollama
    verificar_e_iniciar_ollama(modelo);

    // Inicializar historial del chat desde el archivo JSON
    inicializar_historial(archivo_json, historial);

    // Definir opciones del modelo
    ollama::options opciones;
    opciones["top_k"] = 40;
    opciones["temperature"] = 0.6;

    // Bucle de tres interacciones
    for (int i = 0; i < 3; i++) {
        std::string prompt;
        std::cout << "Tú: ";
        std::getline(std::cin, prompt);
        
        generar_respuesta(prompt, historial, modelo, opciones);
    }

    return 0;
}
