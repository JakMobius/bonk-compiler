# Bonk Compiler

## Overview
The Bonk Compiler is a toy project developed as part of an MIPT course. It compiles programs written in the BonkScript language into x86 mach-o object files

This language is not intended to be used in production, it is just a toy language for educational purposes. That's why it has such an inconvenient, but rather funny syntax. 

## Requirements
- `cmake`
- `gcc`

## Build
To build the Bonk Compiler, run the following commands:

```bash
git clone https://github.com/jakmobius/bonk-compiler
mkdir build
cd build
cmake ..
make
cd ..
```

## Usage
To compile a BonkScript program, run the following command:

```bash
build/bonk <path-to-file>
```

Right now compiler is not in a working condition, but theoretically, one could use `-t` flag to switch between backends.

```bash
build/bonk <path-to-file> -t x86 # to use x86 backend 
```

First backend to be implemented is qbe. To use it, run the compiler with `-t qbe` flag.

## Example

To see an example of a program written in BonkScript, see the `Grammar Reference.pdf` in the root of the repository.
It also contains the language grammar.