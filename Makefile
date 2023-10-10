CC=gcc
texture: texture.c
	${CC} texture.c -o build/texture -Wall -Wextra -pedantic -std=c99

run:
	./build/texture texture.c

valgrind:
	valgrind -s --leak-check=full --show-leak-kinds=all build/texture texture.c 