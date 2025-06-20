CC=gcc
CFLAGS=-Wall -std=c11 -pedantic
TARGET=build/enigma
SRC=enigma.c

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

