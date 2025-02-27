#include "json_parser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::unordered_map<std::string, std::string> JsonParser::loadOptions(const std::string& filename) {
    std::unordered_map<std::string, std::string> options;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return options;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            options[key] = value;
        }
    }
    return options;
}
