CC=gcc
texture: texture.c
	${CC} texture.c -o build/texture -Wall -Wextra -pedantic -std=c99

run:
	./build/texture texture.c