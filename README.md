# Human passwords
I suck at remembering random information, like random passwords. For fun and curiosity, i made this tool to generate passphrases from an arbitrary given word pool (for example, containing the english language).

## Building
Presuming you have gcc installed, you can just run `make` in the parent directory of the repository. The executable will appear in a new folder `bin/executable/human-passwords`. Otherwise, you can open and edit the makefile to use your compiler and flags of your choice. Everything needed for compiling and and linking is in the `src/` directory.

## How to use

For general directions and help for specific commands, you can always use `-h` or `--help` in the command line.

Generating passphrases follows 2 steps: first, you need a word pool in the form of a plain text file, where every word is separated by a newline character. For the purpose of testing, i used [this](https://github.com/sloppmachine/wiktionary-extractor).

```human-passwords build --source=dump.txt --target=wordpool.bin```

Then, after having generated a file called `wordpool.bin`, you can exract passwords like:

```human-passwords extract --source=wordpool.bin --target=passwords.txt --amount=10x10```

You can also use a mode called `restore`, which restores the original plain text word pool from the compressed binary:

```human-passwords restore --source=wordpool.bin --target=restored-dump.txt```

The final mode `strength` gives a short summary on the strength of the passphrases generated from a binary:

```human-passwords strength --source=wordpool.bin```

## Problems and future considerations
- The program is not optimized for extraction of large amounts of words (say, over 1000). I might fix that.
- The program currently operates using only the 26 lowercase standard english letters. I might change that.

## Source code documentation
For a summary on project structure, see [conventions.md](conventions.md).

## Licensing
[This work is licensed under the EUPL.](LICENSE.txt)