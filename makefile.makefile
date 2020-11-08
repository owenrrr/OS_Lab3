all:
	nasm -f elf -o my_print.o my_print.asm
	g++ -g -std=c++11 main.cpp my_print.o -o main
