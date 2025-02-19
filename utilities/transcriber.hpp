#ifndef TRANSCRIBER_HPP
#define TRANSCRIBER_HPP

#include <vector>
#include <string>
#include "whisper.h"

class Transcriber {
private:
    struct whisper_context *ctx;
    const char* audioFile;
    const char* recordCommand;
    const char* stopCommand;

public:
    Transcriber(const std::string &modelPath, const std::string &audioPath = "audio.wav");
    ~Transcriber();

    void start_microphone();
    void stop_microphone();
    std::string transcribe_audio();
    
private:
    std::vector<float> load_audio(const std::string &filename);
    int kbhit();
};

#endif // TRANSCRIBER_HPP
