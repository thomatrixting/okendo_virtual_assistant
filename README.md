# Okendo Virtual Assistant (OVA)

OVA is a local virtual assistant running on C++ for most Linux distributions maded for the course of "programacion e introduccion a los metodos numericos" given by [Profesor Oquendo](https://co.linkedin.com/in/william-fernando-oquendo-pati%C3%B1o-a3569b18)
of the Universidad Nacional de Colombia. It promise to give and interective assitant experience as its capable to interpret voice or text input and response close to realitime also in test or voice, eveything wihile only ussing your local resurces of the pc is on so it doen't requiere intenet. It uses Whisper, Espeak and DeepSeek

it has capabilities to respond rapidly to simple questions so you can remeber the sintax and the especific usecases of almost any command on the linux terminal interface. you can try it yourself by typing on your terminal amfq or ova amfq and then typing you question for example 

comand: amfq what comand i can use to search the pattern "model_file" on .txt files

out:
Ollama ya está en ejecución.
================ Asistant out ================

You could use grep command for this purpose which stands for "global regular expression print". It's used in Unix/Linux systems and it searches a text or file based upon given patterns, by default searching through standard input if no other device is specified as an argument to the grep program. 

Here are some examples:
- To search model_file within all .txt files present at your current directory (and subdirectories):
`bash
find . -name "*.txt" | xargs grep 'model_file'
`
This command will find and print out every line in each file named .txt that contains the word/phrase model_file, recursively across all directories starting from your current directory (i.e., it includes subdirectories). 
- To search for a specific .txt files:
- Suppose you have one text file called 'model' and want to find occurrences of model_file in this particular '.txt'. You can use the following command :
`bash
grep 'model_file' model.txt  # assuming your txt is named as "model" with extension .txt, replace it accordingly if necessary  
`    or for a specific file:    
` bash     
grep -I 'your pattern here.' filename      -i flag makes the search case insensitive (default in most systems) and you can use ^ to specify beginning of line.  For example :  grep

==========================================================



## Installation

To install OVA, follow these steps:

1. Make sure you have the necessary dependencies installed.
2. Make the installation script executable and run it:

   ```bash
   chmod +x install.sh
   ./install.sh
   ```

## Setup (After Restarting the Terminal)

Afterward you will need to run setup.sh evertime you log in:

```bash
chmod +x setup.sh
./setup.sh $(pwd) #if you are on the root directory of the proyect or just past the explicit like /home/thomas/Desktop/okendo_virtual_assistant
source [proyect root directory]/commands/.bashrc #to enable the comands

```
you can also add the last 2 like to you local ~/.bashrc so it works everytime you open the terminal

and that it your are redy to use ova

## Usage and examples

## Dependencies
we are ussing for this proyect 
espeak V1.52.0
alsa-utils from the linux kernel


