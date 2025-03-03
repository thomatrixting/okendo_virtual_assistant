# Okendo Virtual Assistant (OVA)

OVA is a local virtual assistant written in C++ for most Linux distributions. It was developed for the course *Programación e Introducción a los Métodos Numéricos*, taught by [Professor Oquendo](https://co.linkedin.com/in/william-fernando-oquendo-pati%C3%B1o-a3569b18) at the Universidad Nacional de Colombia.

OVA aims to provide an interactive assistant experience, capable of interpreting both voice and text inputs and responding in near real-time via text or voice. It operates entirely on local resources, meaning it does not require an internet connection. The assistant leverages **Whisper, Espeak, and DeepSeek** for speech processing and synthesis.

### Features
- **Fast response to simple questions**
- **Assistance with Linux terminal commands**
- **Offline functionality**
- **Voice and text-based interactions**

## Example Usage

You can quickly query OVA by typing the following command in your terminal:

```bash
amfq "What command can I use to search for the pattern 'model_file' in .txt files?"
```

**Example Output:**
```
Ollama is already running.
================ Assistant Output ================

You can use the `grep` command for this purpose, which stands for "global regular expression print." It is used in Unix/Linux systems to search for text patterns in files. By default, it searches through standard input unless specified otherwise.

Here are some examples:
- To search for 'model_file' within all `.txt` files in your current directory and subdirectories:
  find . -name "*.txt" | xargs grep 'model_file'
  This command finds and prints every line in each `.txt` file containing the word 'model_file'.

- To search within a specific `.txt` file:
  grep 'model_file' model.txt
  If your file is named differently, replace `model.txt` accordingly.

- To perform a case-insensitive search:
  grep -i 'your pattern here' filename.txt
==========================================================
```

## Installation

To install OVA, follow these steps:

1. Ensure that all necessary dependencies are installed.
2. Make the installation script executable and run it:
   ```bash
   chmod +x install.sh
   ./install.sh
   ```

After restarting your terminal, you need to run `setup.sh` every time you log in:

```bash
chmod +x setup.sh
./setup.sh [project root directory]  # Run this from the root directory of the project or specify the full path, e.g., /home/thomas/Desktop/okendo_virtual_assistant
source [project root directory]/commands/.bashrc  # Enable the commands
```

To avoid running the last two commands manually each time, you can add them to your local `~/.bashrc` file:

```bash
echo 'source [project root directory]/commands/.bashrc' >> ~/.bashrc
```

if you are on the root of the project you can run also
```bash
   chmod +x install.sh
   ./install.sh
   chmod +x setup.sh
   ./setup.sh $(pwd)
   $(pwd)"/commands/.bashrc"

```

Now you're ready to use OVA!

## Dependencies

This project uses:
- **[Espeak](https://github.com/espeak-ng/espeak-ng) v1.52.0 (latest)** (speech synthesis)
- **[Alsa-utils](https://web.git.kernel.org/)** (audio utilities from the Linux kernel)
- **[Ollama v0.5.13-rc5](https://ollama.com/download/ollama-linux-amd64-rocm.tgz) (latest)** (for handeling llm models on your pc)
- **[deepseek-coder1.3b](https://huggingface.co/deepseek-ai/deepseek-coder-1.3b-instruct) (from deepseek)** (the llm that powers the whole proyect)
- **[ollama-hpp](https://github.com/jmont-dev/ollama-hpp) v0.9.4(latest) (from jmont-dev)** (to handle ollama from c++)
- **[whisper.cpp v1.7.4](https://github.com/ggerganov/whisper.cpphttps://github.com/ggerganov/whisper.cpp)(latest)** (to interpret voice into text)
- **[ggml-base.bin](https://huggingface.co/ggerganov/whisper.cpp/blob/main/ggml-base.bin)** (the especific model of whisper cpp that makes the transcription)

## Usage and Examples

Once installed and set up, you can start using OVA by running:

```bash
amfq "your query here"
```

For example:
```bash
amfq "How do I list all running processes?"
```

### Command-Line Options

The `amfq` command supports additional options:

- `-d`: Requests a detailed response from the assistant.
- `--help`: Displays usage information.

#### Example Commands:

- **Basic Query:**
  ```bash
  amfq "What is the command to list files in a directory?"
  ```
  This will return a short and precise response.

- **Detailed Response:**
  ```bash
  amfq -d "Explain how to use grep for searching patterns in files."
  ```
  This will provide a more in-depth explanation.

- **Help Menu:**
  ```bash
  amfq --help
  ```
  This displays information about how to use the command.

### Compilation and Execution

To compile `amfq`, use the following command:

```bash
g++ -std=c++17 -fsanitize=undefined ask_the_model.cpp ../utilities/call_the_model.cpp -o amfq.out -g
```

Then, run it as:

```bash
./amfq.out "your query here"
```

OVA will provide an immediate response based on the best matching Linux command.

---

Feel free to contribute or report issues to improve OVA!

