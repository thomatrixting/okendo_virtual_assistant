#include <iostream>
#include "microkey.hpp"  // Include the header file of microkey

int main() {
    // Initialize Whisper with the correct parameters

    AudioFile audioFile = "audios/audio.wav";
    struct whisper_context_params wparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params("../whisper.cpp/models/ggml-base.bin", wparams);
    
    if (!ctx) {
        std::cerr << "❌ Error: No se pudo cargar el modelo Whisper." << std::endl;
        return 1;
    }

    start_microphone(audioFile);  // Wait for 'R' to start recording
    stop_microphone();   // Wait for 'S' to stop recording
    std::cout << "✅ Grabación finalizada. Archivo guardado en " << audioFile << std::endl;

    transcribe_audio(audioFile); // Transcribe recorded audio

    whisper_free(ctx); // Free Whisper memory
    return 0;
}