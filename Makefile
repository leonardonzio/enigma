CC=gcc
CFLAGS=-Wall -std=c99 -pedantic

all: build/enigma

build/enigma: enigma.c
	@mkdir -p build
	$(CC) $(CFLAGS) enigma.c -o build/enigma

run: build/enigma
	./build/enigma

clean:
	rm -f build/enigma

