#include <iostream>
#include <fstream>
#include <vector>
#include "../utilities/json.hpp"
#include "../utilities/ollama.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <limits.h>
#include <filesystem>
#include <string>

// Alias para el namespace JSON
using json = nlohmann::json;

// Estructura para almacenar los mensajes del historial
struct Mensaje {
        std::string role;
        std::string content;
    };

    // Función para verificar si Ollama está corriendo e iniciarlo si es necesario

void verificar_ollama(const std::string& modelo);

void obtener_respuesta(
            ollama::messages& historial, 
            const std::string& modelo, 
            const ollama::options& opciones,
            const std::string& initial_instruction,
            const std::string& prompt,
            const std::string& speaking_role
        );

void inicializar_historial(const std::string& ruta_json, ollama::messages& historial);
void inicializar_opciones(const std::string& ruta_json, ollama::options& opciones);
void print_formatted_output(const std::string& input);
void guardar_en_log(const std::string& usuario, const std::string& mensaje, const std::string& respuesta, bool esError);
void truncar_historial(ollama::messages& historial, int limit);
void format_response_for_audio(const std::string& input, std::string &output);
void reiniciar_servidor();

std::string get_commands_directory();
/*
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
*/
void verificar_ollama(const std::string& modelo) {
    if (!ollama::is_running()) {
        std::cout << "Ollama no está corriendo. Intentando iniciar...\n";
        
        std::string comand_dir = get_commands_directory();
        // Intentar ejecutar el script setup.sh
        std::string set_up_loc = comand_dir + "../setup.sh";
        int resultado = system( set_up_loc.c_str() );
        if (resultado == 0) {
            std::cout << "Script 'setup.sh' ejecutado exitosamente.\n";
            
            // Intentar cargar el modelo después de ejecutar el script
            if (ollama::load_model(modelo)) {
                std::cout << "Modelo '" << modelo << "' cargado exitosamente.\n";
            } else {
                std::cerr << "Error: No se pudo cargar el modelo '" << modelo << "'.\n";
                exit(1);
            }
        } else {
            std::cerr << "Error: No se pudo ejecutar el script 'setup.sh'. Código de retorno: " << resultado << "\n";
            exit(1);
        }
    } else {
        std::cout << "Ollama ya está en ejecución.\n";
    }
}

void obtener_respuesta(
    ollama::messages& historial, 
    const std::string& modelo, 
    const ollama::options& opciones,
    const std::string& initial_instruction,
    const std::string& prompt,
    const std::string speaking_role
)
{
    // Clonar historial
    ollama::messages mensajes = historial;
    mensajes.push_back({"system", initial_instruction});
    mensajes.push_back({speaking_role, prompt});

    // Truncar el historial si es necesario
    truncar_historial(mensajes, 2);

    try {
        // Generar respuesta del modelo
        std::string respuesta = ollama::chat(modelo, mensajes, opciones);

        // Agregar al historial
        historial.push_back({speaking_role, prompt});  
        historial.push_back({"assistant", respuesta}); 

        // Guardar en el log
        guardar_en_log(speaking_role, prompt, respuesta, false);

    } catch (const std::exception& e) {
        std::cerr << "un error en la generacion ha ocurrido se reinciara el servidor" << "\n";
        reiniciar_servidor();
        // Guardar error en log
        guardar_en_log(speaking_role, prompt, e.what(), true);
        std::cerr << "Error critico en la generación de respuesta: " << e.what() << std::endl;
        exit(1);
    }
}

void guardar_en_log(const std::string& usuario, const std::string& mensaje, const std::string& respuesta, bool esError)
{
    std::string logs_dir = get_commands_directory() + "/../logs";
    std::string log_file_path = logs_dir + "/logs_of_messaging.log";

    try {
        // Asegurar que el directorio de logs exista
        std::filesystem::create_directories(logs_dir);

        // Abrir el archivo en modo de agregar (`append`)
        std::ofstream log_file(log_file_path, std::ios::app);
        if (!log_file) {
            std::cerr << "Error: No se pudo abrir el archivo de log: " << log_file_path << std::endl;
            return;
        }

        // Escribir en el log
        log_file << "==== Nueva Entrada ====" << std::endl;
        log_file << "Usuario (" << usuario << "): " << mensaje << std::endl;

        if (esError) {
            log_file << "⚠️ ERROR: " << respuesta << std::endl;
        } else {
            log_file << "Asistente: " << respuesta << std::endl;
        }

        log_file << "======================" << std::endl;

        log_file.close();
    } catch (...) {
        std::cerr << "Error adicional al escribir en el log." << std::endl;
    }
}

void truncar_historial(ollama::messages& historial, int limit) {
    if (historial.size() <= limit*3) {
        return; // Keep history if there are 4 or fewer messages
    }

    ollama::messages nuevo_historial;
    int user_count = 0;

    // Iterate backwards to find the last 'limit' user messages and their responses
    for (auto it = historial.rbegin(); it != historial.rend(); ++it) {
        if (it->contains("role") && it->at("role") == "user") {
            user_count++;
        }
        nuevo_historial.push_back(*it);
        if (user_count == limit) {
            break;
        }
    }

    // Reverse back to correct order
    std::reverse(nuevo_historial.begin(), nuevo_historial.end());

    historial = nuevo_historial;
}

void reiniciar_servidor() {
    std::string script_path = get_commands_directory() + "/../restart_server.sh";
    std::string comand = script_path +" "+ get_commands_directory() + "/../"; //arguments for the script


    // Check if the script exists before running
    if (!std::filesystem::exists(script_path)) {
        std::cerr << "Error: No se encontró el archivo " << script_path << std::endl;
        return;
    }

    // Execute the restart script
    int resultado = std::system(comand.c_str());

    if (resultado == 0) {
        std::cout << "✅ Servidor reiniciado exitosamente con " << script_path << std::endl;
    } else {
        std::cerr << "❌ Error: No se pudo reiniciar el servidor. Código de salida: " << resultado << std::endl;
    }
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

void format_response_for_audio(const std::string& input, std::string &output) {
    std::istringstream stream(input);
    std::string line;
    output.clear();
    int line_count = 0;

    while (std::getline(stream, line)) {
        if (line_count > 0) {
            output += "<|jump_line|>"; // Add jump line tag between lines
        }
        
        for (size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '`') {
                if (i + 2 < line.size() && line[i + 1] == '`' && line[i + 2] == '`') {
                    output += "<|long_comand|>";
                    i += 2; // Skip the next two backticks
                } else {
                    output += "<|short_comand|>";
                }
            } else {
                output += line[i];
            }
        }

        line_count++;
    }
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

// Function that always points to the /commands directory relative to ROOT_DIR
std::string get_commands_directory() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        std::string path(result, count);
        // Forcefully target the parent directory where the executable is stored
        return std::filesystem::path(path).parent_path().string();
    }
    return "";
}