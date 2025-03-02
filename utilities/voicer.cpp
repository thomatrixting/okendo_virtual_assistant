#include "../utilities/voicer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdio> 
#include <filesystem>

//Logging error and success messages from other functions
void logMsg(const std::string& message) {
    
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "voicer.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "❌ Error: No se pudo abrir el archivo de registro en " << logFilePath << std::endl;
    }
}

Voicer::Voicer(std::string archivo, std::string audio)
    : archivoTexto(std::move(archivo)), archivoAudio(std::move(audio)) {}

// Function to capture text based on a mapping maded in the prompt.
void Voicer::capturarTexto() {
    std::ostringstream output;
    std::string line;
    CaptureState state = WAITING;

    while (std::getline(std::cin, line)) {
        if (line == "<|jump_line|>") {
            if (state == WAITING) {
                state = CAPTURING; // Start capturing
            } else if (state == CAPTURING) {
                output << "<|jump_line|>"; // Append the tag
                if (!std::getline(std::cin, line) || line.empty()) {
                    state = WAITING;
                    break;
                }
                output << line; // Append the next valid line
            }
            continue;
        }

        if (line == "<|short_command|>") {
            continue; // Ignore this tag
        }

        if (line == "<|long_command|>") {
            state = (state == CAPTURING) ? PAUSED : CAPTURING;
            continue;
        }

        if (state == CAPTURING) {
            output << line;
        }
    }

    std::string textoCapturado = output.str();  // Capture the output text
    generarAudio(textoCapturado);

}

//Function that creates a temporary file with the mapped prompt text and generates audio with eSpeak NG.
void Voicer::generarAudio(const std::string &texto) {
    if (texto.empty()) {
        std::string errMsg = "Warning: No text provided for audio generation.";
        logMsg(errMsg);
        return;
    }

    // Create a temporary file to store the text
    std::string tempFile = "/tmp/voicer_text.txt";
    std::ofstream outFile(tempFile);
    if (!outFile) {
        std::string errMsg = "❌ Error: Could not create temporary file for text input.";
        logMsg(errMsg);
        return;
    }
    outFile << texto;
    outFile.close();

    // Construct the espeak command using the temp file
    std::ostringstream comando;
    comando << "espeak -w " << archivoAudio
            << " --stdout -f " << tempFile << " | aplay > /dev/null 2>&1";

    // Execute the command safely
    FILE* pipe = popen(comando.str().c_str(), "r");
    if (!pipe) {
        std::string errMsg = "❌ Error: Failed to execute audio command.";
        logMsg(errMsg);
        return;
    }
    pclose(pipe);

    // Remove the temporary file
    std::remove(tempFile.c_str());
}
