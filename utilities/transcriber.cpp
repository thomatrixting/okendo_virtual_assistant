#include "transcriber.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>  // Para exit()

// Constructor: inicializa Whisper usando la API encapsulada con macros.
Transcriber::Transcriber(const std::string &modelPath, const std::string &audioPath) {
    ParametrosWhisper wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params(modelPath.c_str(), wparams);

    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        exit(1);
    }

    // Se guarda la ruta del archivo de audio y se asignan los comandos para grabar/detener.
    audioFile = audioPath.c_str();
    recordCommand = "arecord -f S16_LE -r 16000 -c 1 audio.wav &"; // Linux (Alsa)
    stopCommand = "pkill -f arecord";
}

// Destructor: libera la memoria del contexto de Whisper.
Transcriber::~Transcriber() {
    whisper_free(ctx);
}

// Función para obtener la configuración de la terminal
void set_terminal_attributes(struct termios& oldt, struct termios& newt) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    DISABLE_CANONICAL_MODE(newt);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

// Función para restaurar la configuración original de la terminal
void restore_terminal_settings(struct termios& oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

// Función para obtener el estado de una tecla sin bloquear
int get_key() {
    return getchar();
}

// Función para gestionar la entrada de teclado sin bloqueo
bool keyboardhit() {
    struct termios oldt, newt;
    int oldf;
    set_terminal_attributes(oldt, newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    SET_NONBLOCKING_MODE();

    int ch = get_key();

    restore_terminal_settings(oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }
    return false;
}

// Inicia la grabación esperando que se pulse 'R'
void Transcriber::start_microphone() {
    std::cout << "🎤 Presiona 'R' para comenzar la grabación..." << std::endl;
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            if (key == 'r' || key == 'R') {
                std::cout << "🎙️ Grabando... Presiona 'S' para detener." << std::endl;
                if (system(recordCommand) != 0) {
                    std::cerr << "❌ Error al iniciar la grabación." << std::endl;
                }
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Evitar alto uso de CPU
    }
}

// Detiene la grabación esperando que se pulse 'S'
void Transcriber::stop_microphone() {
    std::cout << "Presiona 'S' para detener la grabación..." << std::endl;
    while (true) {
        if (keyboardhit()) {
            char key = getchar();
            if (key == 's' || key == 'S') {
                std::cout << "🛑 Deteniendo grabación..." << std::endl;
                if (system(stopCommand) != 0) {
                    std::cerr << "❌ Error al detener la grabación." << std::endl;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Evitar alto uso de CPU
    }
}

// Carga el archivo WAV y lo convierte a un vector de float (normalizado)
std::vector<float> Transcriber::load_audio(const std::string &filename) {
    OPEN_WAV_FILE(filename, file);  // Abre el archivo WAV

    file.seekg(WAV_HEADER_SIZE);  // Omitir cabecera WAV
    std::vector<int16_t> samples;
    int16_t sample;
    READ_SAMPLES(file, sample) {  // Leer muestras
        samples.push_back(sample);
    }
    file.close();

    std::vector<float> audioData(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        audioData[i] = NORMALIZE_SAMPLE(samples[i]);  // Normalizar a float
    }

    return audioData;
}

// Transcribe el audio usando la API de Whisper (a través de macros)
std::string Transcriber::transcribe_audio() {
    std::vector<float> audioData = load_audio(audioFile);
    if (audioData.empty()) {
        std::cerr << "❌ No se pudo cargar el audio." << std::endl;
        return "";
    }

    WhisperConfig params = whisper_crear_parametros(WHISPER_SAMPLING_GREEDY);
    params.language = "es";      // Establecer el idioma
    params.print_progress = false;  // Deshabilitar el progreso

    if (whisper_full(ctx, params, audioData.data(), audioData.size()) != 0) {
        std::cerr << "❌ Error al transcribir el audio." << std::endl;
        return "";
    }

    int num_segments = whisper_num_segmentos(ctx);
    std::string transcript;
    for (int i = 0; i < num_segments; ++i) {
        transcript += whisper_obtener_texto_segmento(ctx, i);
        transcript += " ";
    }

    return transcript;
}

