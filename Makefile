CC=gcc
CFLAGS=-Wall -std=c99 -pedantic

all: build/enigma

build/enigma: enigma.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o build/enigma enigma.c 

build/enigma.so: enigma.c
	@mkdir -p build
	$(CC) $(CFLAGS) -fPIC -shared -o build/enigma.so enigma.c

shared: build/enigma.so

run: build/enigma
	./build/enigma

clean:
	rm -f build/enigma
