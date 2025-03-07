#include <cstdio>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include "../utilities/voicer.hpp"

Voicer::Voicer(std::string file, std::string audio) : textFile(std::move(file)), audioFile(std::move(audio)) {}

// Functions



// Function to capture text based on a mapping maded in the prompt.
void Voicer::captureText() {
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

    std::string capturedText = output.str();  // Capture the output text
    generateAudio(capturedText);

}

//Logging error and success messages from other functions
void Voicer::voicerlog(const std::string& message) {
    
    std::string logDirectory = "../logs/"; 
    std::filesystem::create_directories(logDirectory);
    std::string logFilePath = logDirectory + "voicer.log";

    std::ofstream logFile(logFilePath, std::ios::app); // Open in append mode
    if (logFile) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << " ❌ Error: Could not open log file in " << logFilePath << std::endl;
    }
}

//Function that creates a temporary file with the mapped prompt text and generates audio with eSpeak NG.
void Voicer::generateAudio(const std::string &text) {
    if (text.empty()) {
        std::string errMsg = "Warning: No text provided for audio generation.";
        voicerlog(errMsg);
        return;
    }

    // Create a temporary file to store the text
    std::string tempFile = "/tmp/voicer_text.txt";
    std::ofstream outFile(tempFile);
    if (!outFile) {
        std::string errMsg = " ❌ Error: Could not create temporary file for text input.";
        voicerlog(errMsg);
        return;
    }
    outFile << text;
    outFile.close();

    // Check if espeak is available
    bool espeakAvailable = (system("espeak --version > /dev/null 2>&1") == 0);
    
    std::string selectedEngine = espeakAvailable ? "espeak" : "espeak-ng";
    
    if (!espeakAvailable) {
        std::string warnMsg = "⚠️ Warning: 'espeak' not found, switching to 'espeak-ng'.";
        voicerlog(warnMsg);
    }

    // Construct the espeak command using the temp file
    std::ostringstream command;
    command << selectedEngine << " -w " << audioFile
            << " --stdout -f " << tempFile << " | aplay > /dev/null 2>&1";

    // Execute the command safely
    FILE* pipe = popen(command.str().c_str(), "r");
    if (!pipe) {
        std::string errMsg = "❌ Error: Failed to execute audio command.";
        voicerlog(errMsg);
        return;
    }
    pclose(pipe);

    // Remove the temporary file
    std::remove(tempFile.c_str());
}