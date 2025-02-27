#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <unordered_map>
#include <string>

class JsonParser {
public:
    /**
     * Loads key-value pairs from a JSON-like configuration file.
     * Each line should be formatted as "key:value".
     * 
     * @param filename The path to the configuration file.
     * @return An unordered_map containing the key-value pairs.
     */
    static std::unordered_map<std::string, std::string> loadOptions(const std::string& filename);
};

#endif // JSON_PARSER_HPP
