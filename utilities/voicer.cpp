#include "voicer.hpp"

Voicer::Voicer(std::string archivo, std::string audio)
    : archivoTexto(archivo), archivoAudio(audio) {}


Voicer CaptureState; // Definimos los posibles estados de la captura.

/*
void Voicer::capturarTexto() {
    std::string line, output;
    CaptureState state = WAITING;
    bool jumpPending = false;  // Indica que se leyó un <|jump_line|> en CAPTURING

    while (std::getline(std::cin, line)) {
        if (line == "<|jump_line|>") {
            if (state == WAITING) {
                // Primera aparición: iniciar captura
                state = CAPTURING;
            } else if (state == CAPTURING) {
                // Se marca la aparición de un salto de línea pendiente
                jumpPending = true;
            }
            // En estado PAUSED se ignoran estas etiquetas
            continue;
        }

        if (line == "<|short_command|>") {
            // Se ignora y se continúa leyendo
            continue;
        }

        if (line == "<|long_command|>") {
            // Alterna entre CAPTURING y PAUSED; se resetea el salto pendiente
            state = (state == CAPTURING) ? PAUSED : CAPTURING;
            jumpPending = false;
            continue;
        }

        // Procesamiento de líneas de texto normales
        if (state == CAPTURING) {
            if (jumpPending) {
                // Si se leyó un <|jump_line|> previamente:
                // - Si la línea actual está vacía, se termina la captura.
                // - De lo contrario, se agrega el salto y se continúa.
                if (line.empty()) {
                    state = WAITING;
                    break;
                }
                output += "<|jump_line|>";
                jumpPending = false;
            }
            output += line;
        }
        // En estados WAITING o PAUSED se ignora el texto normal
    }

    std::ofstream fout(archivoTexto);
    if (!fout) {
        std::cerr << "❌ Error: No se pudo abrir el archivo para escritura." << std::endl;
        return;
    }
    fout << output;
}
*/

void Voicer::capturarTexto() {
    std::string line;
    std::string output;
    CaptureState state = WAITING;
    int line_count = 0; // Contador de líneas (se incrementa con cada tag reconocida)

    while (std::getline(std::cin, line)) {
        // Si se encuentra la tag <|jump_line|>
        if (line == "<|jump_line|>") {
            if (state == WAITING) {
                // La primera vez: se inicia la captura
                state = CAPTURING;
                line_count++;
                // No se escribe nada en output para esta tag de inicio
            }
            else if (state == CAPTURING) {
                // Mientras se esté capturando, se añade el tag a la salida
                output += "<|jump_line|>";
                line_count++;

                // Se hace un "lookahead": se lee la siguiente línea para ver si trae texto.
                if (!std::getline(std::cin, line))
                    break; // Fin de entrada
                if (line.empty()) {
                    // Si después de la tag no hay texto, se finaliza la captura
                    state = WAITING;
                    break;
                } else {
                    // Si hay texto, se procesa esa línea normalmente en el estado actual.
                    // (Se continúa en CAPTURING)
                    output += line;
                }
            }
            else if (state == PAUSED) {
                // En estado PAUSED la tag <|jump_line|> no activa la captura,
                // pero se sigue contando la línea.
                line_count++;
            }
            continue;
        }
        // Si se encuentra la tag <|short_command|>
        else if (line == "<|short_command|>") {
            // No se altera el estado; se continúa leyendo.
            continue;
        }
        // Si se encuentra la tag <|long_command|>
        else if (line == "<|long_command|>") {
            // Si se estaba capturando, se pausa la captura.
            // Si ya estaba en pausa, se reanuda.
            if (state == CAPTURING)
                state = PAUSED;
            else if (state == PAUSED)
                state = CAPTURING;
            line_count++;
            continue;
        }
        else {
            // Si la línea es texto normal y estamos en estado CAPTURING, se agrega a la salida.
            if (state == CAPTURING) {
                output += line;
            }
            // En estados WAITING o PAUSED se ignora el texto.
        }
    }

    // Se escribe el texto capturado (y formateado) en el archivo.
    std::ofstream fout(archivoTexto);
    if (!fout) {
        std::cerr << "❌ Error: No se pudo abrir el archivo para escritura." << std::endl;
        return;
    }
    fout << output;
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
