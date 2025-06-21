#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    int position;
    char *name;
} Rotor;

// https://www.ciphermachinesandcryptology.com/en/enigmatech.htm
static Reflector ALL_REFLECTORS[] = {
    { .wiring= "YRUHQSLDPXNGOKMIEBFZCWVJAT", .name = "Reflector B" },
    { .wiring= "FVPJIAOYEDRZXWGCTKUQSBNMHL", .name = "Reflector C" }
};

// https://www.codesandciphers.org.uk/enigma/rotorspec.htm
static Rotor ALL_ROTORS[] = {
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .position = 0, .name = "Rotor I" },  // Right rotor (first in signal path)
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .position = 0, .name = "Rotor II" },   // Middle rotor
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .position = 0, .name = "Rotor III" } // Left rotor (last in forward path)
};

/* --------- functions */


char encrypt_char(char c, Rotor rotors[], Reflector reflector) {
    
    printf("Right: %s\t%s\n", rotors[0].name, rotors[0].wiring);
    printf("Middle: %s\t%s\n", rotors[1].name, rotors[1].wiring);
    printf("Left: %s\t%s\n", rotors[2].name, rotors[2].wiring);
    printf("Reflector:%s\t%s\n", reflector.name, reflector.wiring);
    printf("\nLetter: %c:\n", c);
    
    // convert to index of the alphabet (0-25)
    int index = (int) c - 'A';
    printf("Input: %c (position %d)\n", c, index);
    
    // forward path: I -> II -> III
    for (int i=0; i<NUM_ROTORS; i++) {
        
        // add the offset of rotor position
        int input_pos = (index + rotors[i].position) % ALPHABET_SIZE;
        
        // ouput character from rotor wiring
        char output_char = rotors[i].wiring[input_pos];
        
        // convert back to index for next rotor
        index = (int)(output_char - 'A');
        printf("Passing in %s:\tposition %d -> %c (position %d)\n", rotors[i].name, input_pos, output_char, index);
    }
    
    // reflector B
    char reflected_char = reflector.wiring[index];
    index = (int)(reflected_char - 'A');
    printf("Passing in reflector B: -> %c (position %d)\n", reflected_char, index);
    
    // backward path: III -> II -> I
    for (int i=NUM_ROTORS - 1; i>=0; i--) {
        
        char input_char = (char)('A' + index);
        
        // find inverse mapping: which input position gives us this output?
        int inverse_pos = 0;
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
   
    Reflector reflector = ALL_REFLECTORS[0]; // Reflector B
    Rotor rotors[NUM_ROTORS] = {
        ALL_ROTORS[0], // Rotor I
        ALL_ROTORS[1], // Rotor II
        ALL_ROTORS[2]  // Rotor III
    };
    
    printf("Test: encrypt character Q:\n");
    char encrypted_char = encrypt_char('Q', rotors, reflector);
    printf("\nEncrypted: Q -> %c\n", encrypted_char);
    
    return 0;
}
