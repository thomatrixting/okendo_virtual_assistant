#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>      // For popen, pclose
#include <cstdio>       // For FILE*, fgets
#include "json.hpp"
#include "ollama.hpp"

using json = nlohmann::json;

//=======================================================
// 1) Data Structures
//=======================================================

// Estructura para almacenar los mensajes del historial
struct Mensaje {
    std::string role;      // "user", "assistant", "system"
    std::string content;   // Texto del mensaje
};

//=======================================================
// 2) Utility Functions
//=======================================================

/**
 * @brief Check if Ollama is running; if not, attempt to load the given model.
 * @param modelo Model name to load, e.g. "deepseek-r1:1.5b"
 */
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

/**
 * @brief Load initial conversation/history from a JSON file.
 * @param ruta_json Path to the JSON file (e.g. "historial_test.json")
 * @param historial Vector of Mensaje to populate
 */
void inicializar_historial(const std::string& ruta_json, std::vector<Mensaje>& historial) {
    std::ifstream archivo(ruta_json);
    if (!archivo) {
        std::cerr << "Error: No se pudo abrir el archivo JSON: " << ruta_json << "\n";
        return;
    }

    json json_data;
    archivo >> json_data;
    
    for (const auto& item : json_data) {
        // Each item must contain "role" and "content"
        Mensaje msg;
        msg.role    = item.at("role").get<std::string>();
        msg.content = item.at("content").get<std::string>();
        historial.push_back(msg);
    }

    std::cout << "Historial cargado desde " << ruta_json << "\n";
}

/**
 * @brief Extract hidden reasoning `<think>...</think>` blocks from a string (for debug only).
 * @param respuesta The entire assistant response
 * @return Concatenated contents of all <think> blocks (or empty string if none)
 */
std::string extraer_think(const std::string& respuesta) {
    std::string contenido;
    const std::string startTag = "<think>";
    const std::string endTag   = "</think>";

    size_t posInicio = 0;
    while (true) {
        size_t s = respuesta.find(startTag, posInicio);
        if (s == std::string::npos) break;

        size_t e = respuesta.find(endTag, s + startTag.size());
        if (e == std::string::npos) break;

        // Move beyond the <think> tag
        s += startTag.size();
        // Accumulate the inside text
        contenido += respuesta.substr(s, e - s) + "\n";
        // Advance position
        posInicio = e + endTag.size();
    }
    return contenido;
}

/**
 * @brief Extract the first `<code>...</code>` block from the response.
 * @param respuesta Assistant's entire message
 * @return The command text inside `<code>...</code>` or empty if not found
 */
std::string extraer_comando(const std::string& respuesta) {
    const std::string startTag = "<code>";
    const std::string endTag   = "</code>";

    size_t startPos = respuesta.find(startTag);
    size_t endPos   = respuesta.find(endTag);

    if (startPos == std::string::npos || endPos == std::string::npos) {
        return "";
    }

    // Move index beyond `<code>`
    startPos += startTag.size();
    // Return text inside the code block
    return respuesta.substr(startPos, endPos - startPos);
}

/**
 * @brief Check if the assistant response contains `<clear>`
 * @param respuesta Entire assistant response
 * @return True if `<clear>` is present, false otherwise
 */
bool contiene_clear(const std::string& respuesta) {
    return (respuesta.find("<clear>") != std::string::npos);
}

/**
 * @brief Execute a shell command with popen() and capture its output.
 * @param cmd Shell command to execute
 * @param return_code Reference to int to store the command's exit code
 * @return The captured stdout output from the command
 */
std::string ejecutar_comando(const std::string& cmd, int& return_code) {
    std::string data;
    FILE* stream = popen(cmd.c_str(), "r");
    if (!stream) {
        return_code = -1;
        return "Error: no se pudo ejecutar el comando.";
    }

    const int MAX_BUFFER = 256;
    char buffer[MAX_BUFFER];
    while (fgets(buffer, MAX_BUFFER, stream) != nullptr) {
        data.append(buffer);
    }

    int status = pclose(stream);
    if (WIFEXITED(status)) {
        return_code = WEXITSTATUS(status);
    } else {
        return_code = -1;
    }

    return data;
}

/**
 * @brief Use the Ollama model to generate an assistant response, given current conversation history.
 * @param historial Vector of Mensaje (conversation so far)
 * @param modelo The model name to use (e.g., "deepseek-r1:1.5b")
 * @param opciones ollama::options (temperature, max_new_tokens, etc.)
 * @return Full text of the assistant's raw response
 */
std::string obtener_respuesta(
    std::vector<Mensaje>& historial, 
    const std::string& modelo, 
    const ollama::options& opciones
) {
    // Convert our history to ollama::messages
    ollama::messages mensajes;
    for (const auto& msg : historial) {
        mensajes.push_back({msg.role, msg.content});
    }

    // Query the model
    std::string respuesta = ollama::chat(modelo, mensajes, opciones);

    // Store the assistant's response in the history
    historial.push_back({"assistant", respuesta});

    return respuesta;
}

//=======================================================
// 3) Main Function (Conversation Flow)
//=======================================================
int main() {
    // Model name and JSON path
    std::string modelo = "deepseek-r1:8b";
    std::string archivo_json = "historial_test.json";

    // Conversation history
    std::vector<Mensaje> historial;

    // 1) Check / start Ollama
    verificar_e_iniciar_ollama(modelo);

    // 2) Load conversation history from JSON
    inicializar_historial(archivo_json, historial);

    // 3) Define model options
    ollama::options opciones;
    opciones["temperature"]    = 0.2;
    opciones["max_new_tokens"] = 150;
    opciones["top_k"]          = 60;

    // 4) Main loop: read user input, get assistant response, parse commands, handle <clear>
    while (true) {
        //--- Get user input ---
        std::cout << "\nTú: ";
        std::string prompt;
        if (!std::getline(std::cin, prompt)) {
            // End if EOF or input error
            break;
        }

        // Store user message
        historial.push_back({"user", prompt});

        //--- Get assistant response ---
        std::string respuesta = obtener_respuesta(historial, modelo, opciones);

        //=== Debug: extract <think> blocks (not shown to user) ===
        std::string bloquesThink = extraer_think(respuesta);
        if (!bloquesThink.empty()) {
            std::cerr << "\n[DEBUG] Razonamiento interno del asistente:\n"
                      << bloquesThink << "\n";
        }

        //=== Show assistant response (as-is, or optionally remove <think> text) ===
        std::cout << "\nAsistente: " << respuesta << "\n";

        //--- Check if assistant included a <code> command ---
        std::string comando = extraer_comando(respuesta);
        if (!comando.empty()) {
            // Execute the command
            std::cout << "\nEjecutando comando: " << comando << "\n";
            int return_code = 0;
            std::string output = ejecutar_comando(comando, return_code);

            // Show system output
            std::cout << "\n[System output]:\n" << output << "\n";

            // Add system output to conversation history so the assistant can see it
            std::string systemContent;
            if (return_code == 0) {
                systemContent = output;
            } else {
                // Include error code in the system message
                systemContent = "Error code: " + std::to_string(return_code) + "\n" + output;
            }
            historial.push_back({"system", systemContent});

            //--- Possibly the assistant will propose a fix if there's an error or omit if success ---
            while (true) {
                // Check if last response had <clear>; if so, done with that command
                if (contiene_clear(respuesta)) {
                    std::cout << "\nEl asistente ha finalizado la tarea con <clear>.\n";
                    break;
                }

                // Generate next response (the assistant might propose a fix)
                respuesta = obtener_respuesta(historial, modelo, opciones);

                // Debug log for <think>
                bloquesThink = extraer_think(respuesta);
                if (!bloquesThink.empty()) {
                    std::cerr << "\n[DEBUG] Razonamiento interno del asistente:\n"
                              << bloquesThink << "\n";
                }

                std::cout << "\nAsistente: " << respuesta << "\n";

                // If we see <clear> now, break
                if (contiene_clear(respuesta)) {
                    std::cout << "\nEl asistente ha finalizado la tarea con <clear>.\n";
                    break;
                }

                // Check if there's another <code> block to run (maybe a fix)
                std::string nuevoComando = extraer_comando(respuesta);
                if (nuevoComando.empty()) {
                    // No new command nor <clear>, so we might continue or break
                    std::cout << "[INFO] No se encontró nuevo comando ni <clear>. Saliendo del bucle de fix.\n";
                    break;
                }

                // Execute the fix attempt
                std::cout << "\nEjecutando comando (intento de corrección): " << nuevoComando << "\n";
                return_code = 0;
                output = ejecutar_comando(nuevoComando, return_code);

                // Display system output
                std::cout << "\n[System output]:\n" << output << "\n";

                // Add new system message
                if (return_code == 0) {
                    systemContent = output;
                } else {
                    systemContent = "Error code: " + std::to_string(return_code) + "\n" + output;
                }
                historial.push_back({"system", systemContent});
            }
        }

        //--- If assistant ended with <clear>, entire conversation can end ---
        if (contiene_clear(respuesta)) {
            std::cout << "\nPrograma finalizado (se recibió <clear>).\n";
            break;
        }
    } // End main while(true)

    return 0;
}
