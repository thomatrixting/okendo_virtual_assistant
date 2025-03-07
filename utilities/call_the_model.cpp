#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits.h>
#include <unistd.h>
#include <filesystem>
#include "../utilities/json.hpp"
#include "../utilities/ollama.hpp"
#include "call_the_model.hpp"

// Abreviation for the namespace JSON
using json = nlohmann::json;

void modelog(const std::string& message) {
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "call_the_model.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << " ❌ Error: Could not open log file in " << logFilePath << std::endl;
    }
}

void verify_ollama(const std::string& model) {
    // Function to verify if Ollama is running and initialize it if needed
    if (!ollama::is_running()) {
        std::cout << "Ollama is not running. Trying to start...\n";
        std::string comand_dir = get_commands_directory();

        // Try to run the script setup.sh
        std::string set_up_loc = comand_dir + "/../setup.sh" + " " + comand_dir + "/../";
        int result = system(set_up_loc.c_str());
        if (result == 0) {
            std::cout << "Script 'setup.sh' successfully executed.\n";
            
            // Try to load the model after running the script
            if (ollama::load_model(model)) {
                std::cout << "Model '" << model << "' loaded successfully.\n";
            } else {
                std::cerr << "Error: Failed to load model '" << model << "'.\n";
                exit(1);
            }
        } else {
            std::cerr << "Error: Failed to execute script 'setup.sh'. Return code: " << result << "\n";
            exit(1);
        }
    } else {
        std::string Msg = "Ollama is running.\n";
        modelog(Msg);
    }
}

void get_response(
    ollama::messages &history, 
    const std::string &model, 
    const ollama::options &options,
    const std::string &initial_instruction,
    const std::string &prompt,
    const std::string &speaking_role
) {
    // Clone history
    ollama::messages messages = history;
    messages.push_back({"system", initial_instruction});
    messages.push_back({speaking_role, prompt});

    // Truncate the history if needed
    truncate_history(messages, 2);

    try {
        // Generate model response
        std::string response = ollama::chat(model, messages, options);

        // Add to history
        history.push_back({speaking_role, prompt});  
        history.push_back({"assistant", response}); 

        // Save in the log
        save_in_log(speaking_role, prompt, response, false);

    } catch (const std::exception& e) {
        std::cerr << "An error occurred during generation. The server will be restarted." << "\n";
        restart_server();
        
        // Save error in the log
        save_in_log(speaking_role, prompt, e.what(), true);
        std::cerr << "Critical error in response generation: " << e.what() << std::endl;
        std::cerr << "Try again later" << e.what() << std::endl;
        exit(1);
    }
}

void save_in_log(const std::string& user, const std::string& message, const std::string& response, bool esError) {
    std::string logs_dir = get_commands_directory() + "/../logs";
    std::string log_file_path = logs_dir + "/logs_of_messaging.log";

    try {
        // Ensure the logs directory exists
        std::filesystem::create_directories(logs_dir);

        // Open the file in append mode
        std::ofstream log_file(log_file_path, std::ios::app);
        if (!log_file) {
            std::cerr << "Error: Could not open the log file: " << log_file_path << std::endl;
            return;
        }

        // Write to the log
        log_file << "======= New Entry =======" << std::endl;
        log_file << "User (" << user << "): " << message << std::endl;

        if (esError) {
            log_file << "⚠️ ERROR: " << response << std::endl;
        } else {
            log_file << "Assistant: " << response << std::endl;
        }

        log_file << "=========================" << std::endl;

        log_file.close();
    } catch (...) {
        std::cerr << "Additional error writing to the log." << std::endl;
    }
}


void truncate_history(ollama::messages &history, int limit)
{
    if (history.size() <= limit*3) {
        return; // Keep history if there are 4 or fewer messages
    }

    ollama::messages new_history;
    int user_count = 0;

    // Iterate backwards to find the last 'limit' user messages and their responses
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (it->contains("role") && it->at("role") == "user") {
            user_count++;
        }
        new_history.push_back(*it);
        if (user_count == limit) {
            break;
        }
    }

    // Reverse back to correct order
    std::reverse(new_history.begin(), new_history.end());

    history = new_history;
}

void restart_server() {
    std::string script_path = get_commands_directory() + "/../restart_server.sh";
    std::string comand = script_path +" "+ get_commands_directory() + "/../"; // Arguments for the script

    // Check if the script exists before running
    if (!std::filesystem::exists(script_path)) {
        std::cerr << "Error: File not found " << script_path << std::endl;
        return;
    }

    // Execute the restart script
    int result = std::system(comand.c_str());

    if (result == 0) {
        std::cout << " ✅ Server restarted successfully with " << script_path << std::endl;
    } else {
        std::cerr << " ❌ Error: Failed to restart server. Exit code: " << result << std::endl;
    }
}

// Function to initialize chat history from a JSON
void initialize_history(const std::string& route_json, ollama::messages& history) {
    std::ifstream file(route_json);
    if (!file) {
        std::cerr << "Error: Could not open JSON file: " << route_json << std::endl;
        return;
    }

    // Read JSON file
    json json_data;
    file >> json_data;
        
    // Convertir a ollama::messages
    for (const auto& item : json_data) {
        if (item.contains("role") && item.contains("content")) {
            history.push_back({ item["role"].get<std::string>(), item["content"].get<std::string>() });
        }
    }

    std::string Msg = "History uploaded from " + route_json + "\n";
    modelog(Msg);
}

void initialize_options(const std::string& route_json, ollama::options& options) {
    std::ifstream file(route_json);
    if (!file) {
        std::cerr << "Error: Could not open JSON file: " << route_json << std::endl;
        return;
    }

    // Read the JSON file
    json json_data;
    file >> json_data;

    // Convert to ollama::options
    for (auto& [key, value] : json_data.items()) {
        if (value.is_number_integer()) {
            options[key] = value.get<int>();
        } else if (value.is_number_float()) {
            options[key] = value.get<double>();
        } else if (value.is_string()) {
            options[key] = value.get<std::string>();
        } else {
            std::cerr << "Warning: key ignored in the JSON, unsupported data type ->" << key << std::endl;
        }
    }

    std::string Msg = "Options loaded from " + route_json + "\n";
    modelog(Msg);
}

/*
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
*/
void print_formatted_output(const std::string& input) {
    std::istringstream stream(input);
    std::string line;

    std::cout << " ================== Asistant out ================== \n\n";

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

    std::cout << "\n ================================================== \n";
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