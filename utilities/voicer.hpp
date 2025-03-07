#ifndef VOICER_HPP
#define VOICER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <fstream>

class Voicer {
public:
    // Enumeración para los estados de captura
    enum CaptureState { WAITING, CAPTURING, PAUSED };

private:
    std::string textFile;
    std::string audioFile;

public:
    // Constructor
    Voicer(std::string archivo = "transcription.txt", std::string audio = "audiogene.wav");

    // Métodos
    void captureText();
    void voicerlog(const std::string& message);
    void generateAudio(const std::string &texto);
};

#endif // VOICER_HPP

