#include <stdint.h>
#include <stdio.h>

/* --------- constants + macros */
#define ALPHABET_SIZE 26
#define NUM_ROTORS 3

/* --------- structs & vars */
typedef char Wiring[ALPHABET_SIZE];

typedef struct {
    Wiring wiring;
    char *name;
} Reflector;

typedef struct {
    Wiring wiring;
    uint8_t position;
    char *name;
} Rotor;

// https://www.ciphermachinesandcryptology.com/en/enigmatech.htm
static Reflector reflectors[] = {
    { .wiring= "YRUHQSLDPXNGOKMIEBFZCWVJAT", .name = "Reflector B" },
    { .wiring= "FVPJIAOYEDRZXWGCTKUQSBNMHL", .name = "Reflector C" }
};

// https://www.codesandciphers.org.uk/enigma/rotorspec.htm
static Rotor rotors[] = {
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .position = 0, .name = "Rotor I" },  // Right rotor (first in signal path)
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .position = 0, .name = "Rotor II" },   // Middle rotor
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .position = 0, .name = "Rotor III" } // Left rotor (last in forward path)
};

/* --------- functions */

char encrypt_char(char c) {
    printf("Rotor I (right):\t%s\n", rotors[0].wiring);
    printf("Rotor II (middle):\t%s\n", rotors[1].wiring);
    printf("Rotor III (left):\t%s\n", rotors[2].wiring);
    printf("Reflector B:\t\t%s\n", reflectors[0].wiring);
    
    printf("\nLetter: %c:\n", c);
    
    // convert to index of the alphabet (0-25)
    uint8_t index = (uint8_t) c - 'A';
    printf("Input: %c (position %d)\n", c, index);
    
    // forward path: I -> II -> III
    for (int i=0; i<NUM_ROTORS; i++) {
        
        // add the offset of rotor position
        uint8_t input_pos = (index + rotors[i].position) % ALPHABET_SIZE;
        
        // ouput character from rotor wiring
        char output_char = rotors[i].wiring[input_pos];
        
        // convert back to index for next rotor
        index = (uint8_t)(output_char - 'A');
        printf("Passing in %s:\tposition %d -> %c (position %d)\n", rotors[i].name, input_pos, output_char, index);
    }
    
    // reflector B
    char reflected_char = reflectors[0].wiring[index];
    index = (uint8_t)(reflected_char - 'A');
    printf("Passing in reflector B: -> %c (position %d)\n", reflected_char, index);
    
    // backward path: III -> II -> I
    for (int i=NUM_ROTORS - 1; i>=0; i--) {
        
        char input_char = (char)('A' + index);
        
        // find inverse mapping: which input position gives us this output?
        uint8_t inverse_pos = 0;
        for (int j=0; j<ALPHABET_SIZE; j++) {
            
            if (rotors[i].wiring[j] == input_char) {
                inverse_pos = j;
                break;
            }
        }
        
        // subtract the rotor position (inverse of rotor rotation)
        index = (inverse_pos + ALPHABET_SIZE - rotors[i].position) % ALPHABET_SIZE;
        
        char output_char = (char)('A' + index);
        
        printf("Passing back in %s:\t%c -> position %d -> %c (position %d)\n", rotors[i].name, input_char, inverse_pos, output_char, index);
    }
    
    char result = (char)('A' + index);
    return result;
}

int main(int argc, char *argv[]) {
    printf("------------------");
    printf("Enigma M3");
    printf("------------------\n");
    printf("Rotors I-II-III, Reflector B, all rotors at position 0\n\n");
    
    printf("Test: encrypt character Q:\n");
    char encrypted_char = encrypt_char('Q');
    printf("\nEncrypted: Q -> %c\n", encrypted_char);
    
    return 0;
}
