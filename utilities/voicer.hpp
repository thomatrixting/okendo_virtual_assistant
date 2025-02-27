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
    std::string archivoTexto;
    std::string archivoAudio;

public:
    // Constructor
    Voicer(std::string archivo = "transcripcion.txt", std::string audio = "audiogene.wav");

    // Métodos
    void capturarTexto();
    void leerArchivo(std::string &texto);
    void generarAudio(const std::string &texto);
};

#endif // VOICER_HPP

