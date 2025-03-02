#ifndef TRANSCRIBER_HPP
#define TRANSCRIBER_HPP

#include <vector>
#include <string>
#include "../utilities/whisper.cpp/include/whisper.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>

// Macros para hacer la API de Whisper más intuitiva.
#define ModeloWhisper struct whisper_context
#define ParametrosWhisper struct whisper_context_params
#define WhisperConfig whisper_full_params
#define whisper_crear_parametros whisper_full_default_params
#define whisper_num_segmentos whisper_full_n_segments
#define whisper_obtener_texto_segmento whisper_full_get_segment_text

// Macros para configurar la terminal (para keyboardhit)
#define DISABLE_CANONICAL_MODE(newt)    (newt.c_lflag &= ~(ICANON | ECHO))
#define RESTORE_TERMINAL_SETTINGS()     (tcsetattr(STDIN_FILENO, TCSANOW, &oldt))
#define SET_NONBLOCKING_MODE()          (fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK))

// Macros para cargar y procesar el archivo WAV
#define WAV_HEADER_SIZE 44         // Tamaño del encabezado WAV
#define MAX_SAMPLE_VALUE 32768.0f  // Valor máximo de un sample normalizado

// Macro para abrir el archivo WAV
#define OPEN_WAV_FILE(filename, file)  \
    std::ifstream file(filename, std::ios::binary); \
    if (!file) { \
        std::cerr << "❌ Error al abrir el archivo de audio." << std::endl; \
        return {}; \
    }

// Macro para leer muestras de 16 bits desde el archivo WAV
#define READ_SAMPLES(file, sample)  \
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) 

// Macro para normalizar la muestra
#define NORMALIZE_SAMPLE(sample)  (sample / MAX_SAMPLE_VALUE)

// Función para obtener la configuración de la terminal
void set_terminal_attributes(struct termios& oldt, struct termios& newt);

// Función para restaurar la configuración original de la terminal.
void restore_terminal_settings(struct termios& oldt);

// Función para obtener el estado de una tecla sin bloquear.
int get_key();

// Declaración de la función keyboardhit.
bool keyboardhit();

class Transcriber {
private:
    ModeloWhisper* ctx;
    std::string audioFile;
    const char* recordCommand;
    const char* stopCommand;

public:
    // Constructor: recibe la ruta del modelo y, opcionalmente, la ruta del archivo de audio.
    Transcriber(const std::string &modelPath, const std::string &audioPath = "audio.wav");
    // Destructor: libera la memoria de Whisper.
    ~Transcriber();

    // Métodos públicos para controlar la grabación y transcribir el audio.
    void start_microphone();
    void stop_microphone();
    std::string transcribe_audio();
    
private:
    // Método para cargar el archivo WAV y convertirlo a un vector de float.
    std::vector<float> load_audio(const std::string &filename);
};

#endif // TRANSCRIBER_HPP