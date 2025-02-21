#include <iostream>
#include "/home/felipeg/okendo_virtual_assistant/utilities/transcriber.hpp"
#include "/home/felipeg/okendo_virtual_assistant/utilities/call_the_model.hpp"  

int main() {
    // Crear el objeto Transcriber con la ruta del modelo
    Transcriber transcriber("/home/felipeg/whisper.cpp/models/ggml-base.bin");

    // Cargar opciones e historial del chatbot
    std::string historial_json = "historial_test.json";  
    std::string opciones_json = "opcions.json";  

    ollama::options opciones;
    inicializar_opciones(opciones_json, opciones);

    ollama::messages historial;
    inicializar_historial(historial_json, historial);

    std::string modelo = opciones["model"];
    std::string initial_instruction = opciones["initial_intrucion"];

    verificar_ollama(modelo);

    while (true) {
        std::cout << "\n🔴 Presiona 'R' para grabar audio y 'S' para detener. Escribe '/bye' para salir." << std::endl;

        // Iniciar y detener la grabación con la transcripción
        transcriber.start_microphone();  // Comienza a grabar cuando se presiona 'R'
        transcriber.stop_microphone();   // Detiene la grabación cuando se presiona 'S'

        std::cout << "✅ Grabación finalizada. Transcribiendo...\n";
        std::string prompt = transcriber.transcribe_audio();

        if (prompt.empty()) {
            std::cerr << "⚠️ No se pudo obtener la transcripción del audio.\n";
            continue;
        }

        std::cout << "📝 Transcripción: " << prompt << std::endl;

        if (prompt == "/bye") {
            std::cout << "Saliendo del chat...\n";
            break;
        }

        // Generar respuesta usando el chatbot
        obtener_respuesta(historial, modelo, opciones, initial_instruction, prompt, "user");
        std::cout << "🤖 Asistente: " << historial.back()["content"] << "\n";
    }

    return 0;
}
