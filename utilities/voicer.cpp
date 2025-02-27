#include "voicer.hpp"

Voicer::Voicer(std::string archivo, std::string audio)
    : archivoTexto(archivo), archivoAudio(audio) {}

void Voicer::capturarTexto() {
    std::ofstream fout(archivoTexto);
    if (!fout) {
        std::cerr << "❌ Error: No se pudo abrir el archivo para escritura." << std::endl;
        return;
    }

    std::string texto;
    bool acot = false;

    while (true) {
        std::getline(std::cin, texto);

        if (texto == "caracterespecial1") {
            acot = true;
            continue;
        }

        if (texto == "caracterespecial2") {
            acot = false;
            break;
        }

        if (acot) {
            fout << texto << "\n";
        }
    }

    fout.close();
}

void Voicer::leerArchivo(std::string &texto) {
    std::ifstream fin(archivoTexto);
    if (!fin) {
        std::cerr << "❌ Error: No se pudo abrir el archivo para lectura." << std::endl;
        return;
    }

    std::string linea;
    while (std::getline(fin, linea)) {
        texto += linea + "\n";
    }

    fin.close();
}

void Voicer::generarAudio(const std::string &texto) {
    std::string comando = "espeak -w " + archivoAudio + " \"" + texto + "\"";
    system(comando.c_str());
}
