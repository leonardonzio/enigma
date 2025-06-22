#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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
    char notch;
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
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .notch = 'Q', .position = 0, .name = "Rotor I" },  // Right rotor (first in signal path)
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .notch = 'E', .position = 0, .name = "Rotor II" },   // Middle rotor
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .notch = 'V', .position = 0, .name = "Rotor III" } // Left rotor (last in forward path)
};

/* --------- functions */

void step_rotors(Rotor rotors[]){
   
    printf("\n---------------STEPPING-------------------\n\n");
    
    bool middle_step = false; // for double step

    // if middle rotor is at notch, step leftest rotor + middle one
    if (rotors[1].position == (rotors[1].notch - 'A')) {
        rotors[2].position = (rotors[2].position + 1) % ALPHABET_SIZE;
        middle_step = true;
    }
    
    // if right rotor is at notch, step middle rotor
    if (rotors[0].position == (rotors[0].notch - 'A')) {
        middle_step = true;
    }

    if (middle_step) {
        rotors[1].position = (rotors[1].position + 1) % ALPHABET_SIZE;
    }

    // step right rotor
    rotors[0].position = (rotors[0].position + 1) % ALPHABET_SIZE;
}


void print_status(Rotor rotors[]){

    for (int i=0; i<NUM_ROTORS; i++){

        printf("Rotor %s:\n", rotors[i].name);
        printf("Wiring: ");
        for(int j=0; j<ALPHABET_SIZE; j++){
            printf("%c", rotors[i].wiring[j]);
        }
        printf("\nNotch: %c\n", rotors[i].notch);
        printf("Position: %d\n", rotors[i].position);
        printf("--------------------\n");
    }

}



char encrypt_char(char c, Rotor rotors[], Reflector reflector) {
    
    int index = (int) c - 'A'; // converted to index (0-25)
    char c_out;
    
    printf("Input: %c (position %d)\n", c, index);
    
    // forward path
    for (int i=0; i<NUM_ROTORS; i++) {
        
        // add the offset of rotor position to the index
        index = (index + rotors[i].position) % ALPHABET_SIZE;
        
        // ouput character from rotor wiring + convert back to index for the next rotor
        c_out = rotors[i].wiring[index];
        index = (int)(c_out - 'A');
    }
    
    // passing in reflector
    c_out = reflector.wiring[index];
    index = (int)(c_out - 'A');
    
    // backward path
    for (int i=NUM_ROTORS - 1; i >= 0; i--) {
        
        // find inverse mapping: which input position gives us this output?
        int inverse_pos = 0;
        for (int j=0; j<ALPHABET_SIZE; j++) {
            
            if (rotors[i].wiring[j] == c_out) {
                inverse_pos = j;
                break;
            }
        }
        
        // subtract the rotor position (inverse of rotor rotation)
        index = (inverse_pos + ALPHABET_SIZE - rotors[i].position) % ALPHABET_SIZE;
        
        c_out = (char)('A' + index);
    }
    
    return c_out;
}

int main(int argc, char *argv[]) {
    
    Reflector reflector = ALL_REFLECTORS[0]; // Reflector B
    Rotor rotors[NUM_ROTORS] = {
        ALL_ROTORS[2],  // Rotor III
        ALL_ROTORS[1], // Rotor II
        ALL_ROTORS[0] // Rotor I
    };

    printf(" _____       _                             __  __ _____\n");
    printf("| ____|_ __ (_) __ _ _ __ ___   __ _      |  \\/  |___ /\n");
    printf("|  _| | '_ \\| |/ _` | '_ ` _ \\ / _` |_____| |\\/| | |_ \\\n");
    printf("| |___| | | | | (_| | | | | | | (_| |_____| |  | |___) |\n");
    printf("|_____|_| |_|_|\\__, |_| |_| |_|\\__,_|     |_|  |_|____/\n");
    printf("               |___/                                   \n");

    print_status(rotors);
    
    step_rotors(rotors); 
    print_status(rotors);
    
    printf("\nTest: encrypt character Q:\n");
    
    char encrypted_char = encrypt_char('Q', rotors, reflector);
    printf("\nEncrypted: Q -> %c\n", encrypted_char);
    
    return 0;
}
