#ifndef CALL_THE_MODEL_HPP
#define CALL_THE_MODEL_HPP

#include <iostream>
#include <vector>
#include "json.hpp"
#include "ollama.hpp"

// Alias para JSON
using json = nlohmann::json;

// Estructura para mensajes
struct Mensaje {
    std::string role;
    std::string content;
};

// Declaración de funciones
void verificar_ollama(const std::string& modelo);
void obtener_respuesta(
    ollama::messages& historial, 
    const std::string& modelo, 
    const ollama::options& opciones,
    const std::string& initial_instruction,
    const std::string& prompt,
    const std::string speaking_role
);
void inicializar_historial(const std::string& ruta_json, ollama::messages& historial);
void inicializar_opciones(const std::string& ruta_json, ollama::options& opciones);
void print_formatted_output(const std::string& input);
void format_response_for_audio(const std::string& input, std::string &output);  
std::string get_commands_directory();

#endif // CALL_THE_MODEL_HPP
