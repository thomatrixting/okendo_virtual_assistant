# Okendo Virtual Assistant (OVA)

OVA is a local virtual assistant written in C++ for most Linux distributions. It was developed for the course *Programación e Introducción a los Métodos Numéricos*, taught by [Professor Oquendo](https://co.linkedin.com/in/william-fernando-oquendo-pati%C3%B1o-a3569b18) at the Universidad Nacional de Colombia.

OVA aims to provide an interactive assistant experience, capable of interpreting both voice and text inputs and responding in near real-time via text or voice. It operates entirely on local resources, meaning it does not require an internet connection. The assistant leverages **Whisper, Espeak, and DeepSeek** for speech processing and synthesis.

### Features
- **Fast response to simple questions**
- **Assistance with Linux terminal commands**
- **Offline functionality**
- **Voice and text-based interactions**

## Quick example

>here are some fast examples to test when your code is ready 

```bash
ova amfq --voice --speak
```
> follow the instruccions speak and ask somthing like how to create a file on linux and wait for the response to apear an be read

**Example Output:**
```
Entering AMFQ Mode. Say or type 'exit' to quit.
You: 🎤 Press 'R' to talk.
🎙️ Recording...
Press'S' to stop.
🛑 Stopping...
you: how to create a file on linux
================ Asistant out ================

You can use several methods (like touch command or Python script) to do this. Here's an example using bash shell and commands line in Linux terminal as follows, assuming that you want to touch the "test_file". 

Method one is creating new empty file with name 'filename':  
`bash   
$ touch filename     #This will create a text document named test on your current directory. If it doesn't exist already then created otherwise opened in an editor like vim, nano etc.. (default).     
#By default if no extension given but file to be made exists so this command is creating one without any issues as well   
` 
Method two uses the touch utility:  
To use a shell script we'll make need.sh and put following line in it - touch new_file`, then run that with :  `chmod +x need.sh && ./need.sh .     (For method one)    If you want to create multiple files at once or using bash loop for creating many file this approach is good but not recommended due syntax and efficiency can be improved in python scripting language like:
`python  #Using Python  
import os     
for i in range(5):     //To make five new_files, change the value as per requirement.   
open('new_file'+str(i), 'a')`         (Opens a file for appending if it exists else creates one)  This works like method one but more efficient with python  
`     

==========================================================
```
## Installation

OVA is a command-line tool designed to work on Linux. The installation relies on the executables `install.sh` and `setup.sh`, which can be found in the root directory of the project after cloning this repository.

### Steps to Install OVA

1. Clone the repository and navigate to the project root:
   ```bash
   git clone [repository URL]
   cd [repository directory]
   ```

2. Make the installation script executable and run it:
   ```bash
   chmod +x install.sh
   ./install.sh
   ```
   **Note:** Ensure that you run this command from the root of the project.

3. Run `setup.sh`, passing the `[project root directory path]` and the `-r` flag (which means "recompile"):
   ```bash
   chmod +x setup.sh
   ./setup.sh [project root directory] -r
   source [project root directory]/commands/.bashrc  # Enable the commands
   ```
   **Explanation:**
   - `setup.sh` initializes the basic requirements for the program to work.
   - It copies example program files to the `commands` directory with all necessary dependencies.
   - It then compiles the files so that users can modify the examples without affecting the main program.
   - If a user wants to update the compiled examples, they must re-run `setup.sh` with the `-r` flag.

### Persistent Setup for Future Sessions

To ensure OVA is available in every session, add the `setup.sh` script (without the `-r` flag) and the source command to your shell configuration file (`.bashrc` or `.zshrc`):

Example for `.bashrc`:
   ```bash
   echo '[project root directory]/setup.sh [project root directory]' >> ~/.bashrc
   echo 'source [project root directory]/commands/.bashrc' >> ~/.bashrc
   ```

### Quick Start

If you are in the root of the project, you can run the following commands to get started:
   ```bash
   chmod +x install.sh
   ./install.sh
   chmod +x setup.sh
   ./setup.sh $(pwd) -r
   source $(pwd)/commands/.bashrc
   ```

Now you're ready to use OVA!


## Dependencies

This project uses the folowing dependeces and they are installed automatically by the set up process:
- **[Espeak](https://github.com/espeak-ng/espeak-ng) v1.52.0 (latest)** (speech synthesis)
- **[Alsa-utils](https://web.git.kernel.org/)** (audio utilities from the Linux kernel)
- **[Ollama v0.5.13-rc5](https://ollama.com/download/ollama-linux-amd64-rocm.tgz) (latest)** (for handeling llm models on your pc)
- **[deepseek-coder1.3b](https://huggingface.co/deepseek-ai/deepseek-coder-1.3b-instruct) (from deepseek)** (the llm that powers the whole proyect)
- **[ollama-hpp](https://github.com/jmont-dev/ollama-hpp) v0.9.4(latest) (from jmont-dev)** (to handle ollama from c++)
- **[whisper.cpp v1.7.4](https://github.com/ggerganov/whisper.cpphttps://github.com/ggerganov/whisper.cpp)(latest)** (to interpret voice into text)
- **[ggml-base.bin](https://huggingface.co/ggerganov/whisper.cpp/blob/main/ggml-base.bin)** (the especific model of whisper cpp that makes the transcription)

## Usage and Examples

## OVA (`ova` Command)


```bash
ova [chat|amfq] [--voice] [--speak]
```

For example:
```bash
ova amfq
```
that will generate the evitoment were you can add the prompt:
```bash
Entering AMFQ Mode. Say or type 'exit' to quit.
You: [your prompt here]
```

**Output:**
```
Entering AMFQ Mode. Say or type 'exit' to quit.
You: how i install a library on python
================ Asistant out ================

To install Python libraries using pip (Python package installer), you can follow these steps. Here are general instructions for installing packages from PyPI and GitHub repositories in both cases where possible, but please note that not all versions of the same name may be available at different URLs or with slightly varying functionality due to various factors such as dependencies on other libraries etc:

**1) Installing a library directly via pip (Python package installer):** 
- Open your terminal. If you are using Linux, use sudo apt install python3` for Python and then type the name of any module or tool that is available in PyPI to get started with it like so:  ```pip3 install numpy matplotlib scikit-learn pandas seaborn jupyter notebook pydot graphviz ``
- If you are using Windows, use Python and then type the name of any module or tool that is available in PyPI to get started with it like so:  ``python3 install numpy matplotlib scikit-learn pandas seaborn jupyter notebook pydot graphviz ``
- If you are using a specific version, use the following command instead of above one. For example if we want to get started with TensorFlow for Python3:  ``pip install tensorflow==2.0`` or simply just 'tensorflow' in case it is available on PyPI by default (it will be installed).

==========================================================
```

### Command-Line Options

The `ova` command supports additional options:

- `--detail`: use a less restrictive setup of `deepseek-coder` so it will take more time but generate beter responses in return.
- `--speak`: it will use espeak to convert the response into audio and play it.
- `--voice`: it will promot a terminal expecting the ussers to press `r` to record and `s` to stop the recording, which afterward it will convert the audio into a promt that will be answer by the model.
 

#### Example Commands:

- **modes**
  with the mode `amfq` and `chat` it will behave as explain in more depth in the next seccion.

- **Basic Response:**
  ```bash
  ova amfq 
  ```
  this will generate a interface were the promt can be put

  ```bash
  Entering AMFQ Mode. Say or type 'exit' to quit.
  You: [your prompt here]
  ```

  the same for chat mode
  ```bash
  ova chat 
  ```
  that will generate
  ```bash
  Entering Chat Mode. Say or type 'exit' to quit.
  You: 
  ```

- **Detail response:**
  is the same that with the basic just that it will take a litle longer but it will generate better responses

  just with
  ```bash
  ova chat --detail
  ova amfq --detail
  ```  

- **Speak mode**
  it will beahave the same but the response will be read

  just with
  ```bash
  ova chat --speak
  ova amfq --speak
  ```  

- **Voice mode**
  it both chat and amfq mode it will generate and interface that will allow to make the commands by voice input

  it is ran with 
  ```bash
  ova chat --voice
  ```  

  the interface is 
  **interface**
  ```
  Entering AMFQ Mode. Say or type 'exit' to quit.
  You: 🎤 Press 'R' to talk.
  ```
  and you just need to follow instruccions

  for example
  **Output:**
  ```
  Entering AMFQ Mode. Say or type 'exit' to quit.
  You: 🎤 Press 'R' to talk.
  🎙️ Recording...
  Press'S' to stop.
  🛑 Stopping...
  you: how i create a file on linux
  ================ Asistant out ================

  You can use touch command to create files or update existing ones. Here are examples for creating and updating two text-based files named "file1" and "file2":

  To Create Files (File 1):
  `bash
  $ touch /path/to/your_directory/file1
  `
  This will generate a new empty file at the specified location with name file1. If you want to create it in specific directory, replace "/path" and "Your Directory Name". For example:  
  To Create Files (File 2):
  `bash
  $ touch /home/user_name/Documents/myDirectory/file2
  `   
  Replace '/home' with your home folder path. If you want to create in the current directory, just use .(dot) like so touch file3 or if it is a new text document then simply provide name of that doc as below:  
  To Create Files (File 1 and File2):
  `bash
  $ touch /home/user_name/.config/file1.txt && touch ./file2.txt
  `   
  In the above command, touch is used to create new files or update existing ones in a directory specified by path (/path). The dot (.) before file name indicates that it should be created at current location of user running this script and not specific folder/directory as mentioned earlier with "/home". If you want different directories then use the full paths.

  ==========================================================

  ```



## Ask My Fun Question (`amfq` Command)


```bash
amfq "your query here" [--options]
```

For example:
```bash
amfq "How do I list all running processes?"
```

**Output:**
```
================ Asistant out ================

You can use different methods to get information about currently active tasks. Here are some common ways you could accomplish this task on Linux using various commands and utilities available in Unix-like operating systems such as bash, csh (c shell), ksh93(ksh) etc.: 
  
1. Using ps command: This is the most basic way to list all running processes by executing a single line of code from terminal or any scripting language you prefer using in your system like Python/Perl, Ruby and so on.. Here's how it works - 'The ps (process status) utility displays information about currently active processes.':
`bash 
# To list all running process:
$ ps auxwww | grep python
`    
2. Using top command : This is a more advanced way to display the current system load, which includes details of each user-level thread including their PID (process ID), %CPU usage and TIME cpu used by them in real time: 
- To start top process viewer use 'sudo apt install htop' command on Ubuntu. On MacOS you can simply type htop into the terminal to get started, or using Homebrew if it is not installed yet run "brew install htop". Then just press Ctrl+a (all) and then click c in order top will start displaying all running processes with their details:
`bash 
# To list process by

==========================================================
```

### Command-Line Options

The `amfq` command supports additional options:

- `-d`: Requests a detailed response from the assistant.
- `--help`: Displays usage information.
- `--detail`: use a less restrictive setup of `deepseek-coder` so it will take more time but generate beter responses in return

#### Example Commands:

- **Detailed Response:**
  ```bash
  amfq -d "Explain how to use grep for searching patterns in files."
  ```
  **Output:**
  ```
  ================ Asistant out ================

  grep` is a command-line utility used primarily on Unix/Linux systems (like Linux) that searches through text or file content based upon user specified criteria, often called "patterns". It's also commonly referred as `find tool of the same name in other contexts. 

  Here are some basic usage examples:

  1- Search for a specific pattern within files/text (e.g., 'hello world') :  
  `bash
  $ grep hello testfile.txt    # searches "hello" inside file named testfile.txt and prints the lines that contain it 
  Hello, World! This is some text... Hello again in a different line

  ==========================================================

  ```

- **Help Menu:**
  ```bash
  amfq --help
  ```
  **Output:**
  ```
  Usage: ./amfq [PROMPT] [-d] [--help]
  PROMPT    The question you want to ask the model.
  -d        Request a detailed response.
  --help    Show this help message.

  ```


## Chat Session (`chat` Command)

The `chat` command allows users to have an interactive conversation with the assistant.

```bash
chat [--options]
```
for example

```bash
chat
```

**Output:**
```
  Tú: how i can create a file on linux
  ================ Asistant out ================

  You can use various commands to create files and directories. Here are some examples for different scenarios you might encounter during your Linux development workloads (like creating text or binary files):

  1) Create an empty File in Unix/Linux using the touch command : 
  `bash
  $ touch filename   # Replace 'filename' with whatever name of file u want to create.
  `
  2) Creating a Directory: You can use mkdir for creating directories, and you need not specify any options if it is created in current directory as the parent folder will be automatically selected by default when using mkdir command : 
  `bash
  $ mkdir dirname   # Replace 'dirname' with whatever name of your new Directory.
  `
  3) Creating a File and Writing to It: You can use echo or cat commands for writing data into the file, if you want create an empty textfile just type : 
  `bash
  $ nano filename   # Replace 'filename' by what ever name of your new Text/File. This will open up Nano editor in terminal where u could write and save it thereafter using ctrl+x to exit the file, then press y for yes when prompted about saving changes into a non-existent or unnamed filename
  ` 
  4) Creating File with Specific Content: You can use echo command :  
  `bash
  $ echo "Hello World" > newfile.txt # This will create 'newfile' as text file and write the content inside it, if you want to overwrite an existing one then remove > filename part from above line 
  # or simply type: nano +x <filename>  for editing mode in Nano editor which is default when using echo command. Then paste your data into that opened window (you can use ctrl+shift+v and enter to past the content). After writing, press Ctrl-X then Y & Enter respectively
  `  or you could also create a file with specific permissions:  
  `bash
  $ touch filename; chmod +x filename # This will make 'filename' as executable. Replace it if needed by your own name of File and permission for the user (you can use u,g,o to specify different users).  For example : sudo nano /etc/sudoers or chown username:groupname directory_path
  `  
  Note that you need root privileges in order to create files on a non-root filesystem. Also note if your file is created with the same name as an existing one, it will overwrite whatever exists there already!  You can use 'nano' or any other text editor for editing these types of tasks and then save & exit when prompted by Nano (or whichever you are using).

  ==========================================================
  Tú: how i create one ussing touch comand call hello.txt
  ================ Asistant out ================

  You can use touch command to create or update an existing file with specific name (in this case "hello.txt"). Here's how you do it in a terminal window using the bash shell on Linux/Unix systems, assuming that your current directory is where you want 'hello.txt'. 

  `bash
  $ touch hello.txt   # This will create an empty file named as specified (in this case "hello.txt"). If there's already a similar name in the same location or parent folder then it would overwrite that, if not created at current directory otherwise with default filename 'touch'. 
  `
  This command is used to update existing files and also create new ones when they don’t exist yet (i.e., you are creating them). If a file named "hello.txt" already exists in the same location or parent folder, it will be overwritten by this one; otherwise if no such name found then 'touch' command creates an empty textfile with that specific filename and path specified at current directory where your terminal is running (i.e., you are creating a new file).

  ==========================================================
  Tú: /bye

```

### Command-Line Options

- `-d`: Starts the chat session with detailed responses.
- `--help`: Displays usage information.

### Example Usage

To start a chat session, run:

```bash
chat
```

For a detailed response mode, use:

```bash
chat -d
```

To exit the chat session, type `/bye`.
