../cmake-build-debug/bonk_compiler_reinvented test.bs -o test.o -t x86
gcc -c main.c
ld test.o main.o -lSystem -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib
#ld test.o -lSystem -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib
./a.out
