#ifndef VOICER_HPP
#define VOICER_HPP

#include <iostream>
#include <fstream>
#include <string>

class Voicer {
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
