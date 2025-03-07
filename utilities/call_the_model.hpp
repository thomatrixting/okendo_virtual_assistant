#ifndef CALL_THE_MODEL_HPP
#define CALL_THE_MODEL_HPP

#include <string>
#include <vector>
#include "json.hpp"
#include <iostream>
#include "ollama.hpp"

// Abreviation for the namespace JSON
using json = nlohmann::json;

// Structure for storing history messages
struct message {
    std::string role;
    std::string content;
};

// Functions
void restart_server();
std::string get_commands_directory();
void modelog(const std::string& message);
void verify_ollama(const std::string& model);
void print_formatted_output(const std::string& input);
void truncate_history(ollama::messages& history, int limit);
void initialize_options(const std::string& route_json, ollama::options& options);
void initialize_history(const std::string& route_json, ollama::messages& history);
void save_in_log(const std::string& user, const std::string& message, const std::string& response, bool esError);
void get_response(
    ollama::messages& history, 
    const std::string& model, 
    const ollama::options& options,
    const std::string& initial_instruction,
    const std::string& prompt,
    const std::string& speaking_role
);

#endif // CALL_THE_MODEL_HPP