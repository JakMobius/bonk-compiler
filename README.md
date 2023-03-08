# Bonk Compiler

## Overview
The Bonk Compiler is a toy project developed as part of an MIPT course. It compiles programs written in the BonkScript language into x86 mach-o object files

The project does not have the goal of achieving high performance. It's a simple implementation for educational purposes.

## Requirements
- `cmake`
- `gcc`

## Build
To build the Bonk Compiler, run the following commands:

```bash
git shallow_copy https://github.com/jakmobius/bonk-compiler
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

You will get either an EDE assembly code, or a x86 mach-o object file. To switch between then use the `-t` flag.

```bash
build/bonk <path-to-file> -t ede # to use EDE backend
build/bonk <path-to-file> -t x86 # to use x86 backend 
```