#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/* --------- constants + macros */
#define ALPHABET_SIZE 26
#define NUM_ROTORS 3

#define INDEX_TO_C(index)   ((char) ('A' + (index))) // da numero dell alfabeto al carattere corrispondente
#define C_TO_INDEX(c)       ((int)  ((c) - 'A')) // da carattere al numero dell alfabeto

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

// Steckerbrett
typedef struct {
    char wiring[ALPHABET_SIZE]; 
} Plugboard;


// https://www.ciphermachinesandcryptology.com/en/enigmatech.htm
static Reflector ALL_REFLECTORS[] = {
    { .wiring= "YRUHQSLDPXNGOKMIEBFZCWVJAT", .name = "Reflector B" },
    { .wiring= "FVPJIAOYEDRZXWGCTKUQSBNMHL", .name = "Reflector C" }
};

// https://www.codesandciphers.org.uk/enigma/rotorspec.htm
static Rotor ALL_ROTORS[] = {
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .notch = 'Q', .position = 0, .name = "Rotor I" },     // Right rotor (first in signal path)
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .notch = 'E', .position = 0, .name = "Rotor II" },    // Middle rotor
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .notch = 'V', .position = 0, .name = "Rotor III" },   // Left rotor (last in forward path)

    { .wiring = "ESOVPZJAYQUIRHXLNFTGKDCMWB", .notch = 'J', .position = 0, .name = "Rotor IV"},
    { .wiring = "VZBRGITYUPSDNHLXAWMJQOFECK", .notch = 'Z', .position = 0, .name = "Rotor V"}
};

static Plugboard PLUGBOARD_CONFIGS[] = {
    { .wiring = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, // no connections
    { .wiring = "ABQDEFGHIJKLMNOPCRSTUVWXYZ" }  // Q swapped with C
};


/* --------- functions */

// e.g. vedo la lettera C alla posizione 10, mando cio che ci sarebbe nell alafabeto alla posizione 10, ovvero J
// (entra C ed esce J)
char enter_playboard(char c, Plugboard plugboard){
    for (int i=0; i<ALPHABET_SIZE; i++){
        if (c == plugboard.wiring[i]) return INDEX_TO_C(i);
    }

    return c;
}


void step_rotors(Rotor rotors[]){
   
    printf("\n---------------STEPPING-------------------\n\n");
    
    bool middle_step = false; // for double step

    // if middle rotor is at notch, step leftest rotor + middle one
    if (rotors[1].position == C_TO_INDEX(rotors[1].notch)) {

        rotors[2].position = (rotors[2].position + 1) % ALPHABET_SIZE;
        middle_step = true;
    }
    
    // if right rotor is at notch, step middle rotor
    if (rotors[0].position == C_TO_INDEX(rotors[0].notch)) {
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
    
    int index = C_TO_INDEX(c);
    char c_out;
    
    printf("Input: %c (position %d)\n", c, index);
    
    // forward path
    for (int i=0; i<NUM_ROTORS; i++) {
        
        // add the offset of rotor position to the index
        index = (index + rotors[i].position) % ALPHABET_SIZE;
        
        // ouput character from rotor wiring + convert back to index for the next rotor
        c_out = rotors[i].wiring[index];
        index = C_TO_INDEX(c_out);
    }
    
    // passing in reflector
    c_out = reflector.wiring[index];
    index = C_TO_INDEX(c_out);
    
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
        
        c_out = INDEX_TO_C(index);
    }
    
    return c_out;
}

int main(int argc, char *argv[]) {
    
    Reflector reflector = ALL_REFLECTORS[0]; // Reflector B
    Rotor rotors[NUM_ROTORS] = {
        ALL_ROTORS[2], // Rotor III
        ALL_ROTORS[1], // Rotor II
        ALL_ROTORS[0]  // Rotor I
    };

    printf(" _____       _                             __  __ _____\n");
    printf("| ____|_ __ (_) __ _ _ __ ___   __ _      |  \\/  |___ /\n");
    printf("|  _| | '_ \\| |/ _` | '_ ` _ \\ / _` |_____| |\\/| | |_ \\\n");
    printf("| |___| | | | | (_| | | | | | | (_| |_____| |  | |___) |\n");
    printf("|_____|_| |_|_|\\__, |_| |_| |_|\\__,_|     |_|  |_|____/\n");
    printf("               |___/                                   \n");

    print_status(rotors);
    
    char c = 'C';
    printf("\nTest: encrypt character %c:\n", c);
    
    c = enter_playboard(c, PLUGBOARD_CONFIGS[1]); // C swapped with Q in plugboard 
    printf("\nNew character after plugboard: %c\n", c);
                                                                    
    step_rotors(rotors); 
    print_status(rotors);
    
    char encrypted_char = encrypt_char(c, rotors, reflector);
    char finalchar = enter_playboard(encrypted_char, PLUGBOARD_CONFIGS[1]);
    
    printf("\nEncrypted: %c -> %c\n", c, finalchar);
    
    return 0;
}
