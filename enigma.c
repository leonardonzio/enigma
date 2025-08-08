#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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
char enter_plugboard(char c, Plugboard plugboard){
    for (int i=0; i<ALPHABET_SIZE; i++){
        if (c == plugboard.wiring[i]) return INDEX_TO_C(i);
    }
    return c;
}


// NOT CORRECT?
void step_rotors(Rotor rotors[]){
    
    printf("\nSTEPPING\n\n");
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
    int i, j;
    printf("-----STATUS-----\n");
    for (i = 0; i < NUM_ROTORS; i++){

        printf("Rotor %s:\n", rotors[i].name);
        printf("Wiring: ");
        for(j = 0; j < ALPHABET_SIZE; j++){
            printf("%c", rotors[i].wiring[j]);
        }
        printf("\nNotch: %c\n", rotors[i].notch);
        printf("Position: %d\n", rotors[i].position);
        printf("--------------------\n");
    }
    printf("----end of status----\n\n");
}


char encrypt_char(char c, Rotor rotors[], Reflector reflector) {
    
    int index = C_TO_INDEX(c);
    int i, j;
    char c_out;
    
    printf("Input: %c (position %d)\n", c, index);
    
    // forward path
    for (i = 0; i < NUM_ROTORS; i++) {
        
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
    for (i = NUM_ROTORS - 1; i >= 0; i--) {
        
        // find inverse mapping: which input position gives us this output?
        int inverse_pos = 0;
        for (j = 0; j < ALPHABET_SIZE; j++) {
            
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
    int nread, i;
    size_t len = 0;
    char *buffer = NULL;
    char *enc_buffer = NULL;

    printf(" _____       _                             __  __ _____\n");
    printf("| ____|_ __ (_) __ _ _ __ ___   __ _      |  \\/  |___ /\n");
    printf("|  _| | '_ \\| |/ _` | '_ ` _ \\ / _` |_____| |\\/| | |_ \\\n");
    printf("| |___| | | | | (_| | | | | | | (_| |_____| |  | |___) |\n");
    printf("|_____|_| |_|_|\\__, |_| |_| |_|\\__,_|     |_|  |_|____/\n");
    printf("               |___/                                   \n");

    print_status(rotors);
  
    printf("Enter word to encrypt: (only uppercase letters, no spaces)\n");
    nread = getline(&buffer, &len, stdin);
    if (nread == -1) {
        perror("Error reading input");
        free(buffer);
        return 1;
    }
    
    // remove newline 
    if (nread > 0 && buffer[nread - 1] == '\n') {
        buffer[nread - 1] = '\0';
    }
    
    enc_buffer = malloc(nread + 1);
    
    for (i = 0; buffer[i] != '\0'; i++) {
        printf("encrypting character: %c\n", buffer[i]);
        char c = enter_plugboard(buffer[i], PLUGBOARD_CONFIGS[1]); // C swapped with Q
        printf("Character after plugboard: %c\n", c);

        step_rotors(rotors);
        print_status(rotors);

        char enc_char = encrypt_char(c, rotors, reflector);
        char final_char = enter_plugboard(enc_char, PLUGBOARD_CONFIGS[1]);

        printf("FINAL encrypted character: %c -> %c\n", c, final_char);
        
        // store it 
        enc_buffer[i] = final_char;
    }

    enc_buffer[i] = '\0';
    printf("\nEncrypted word: %s\n", enc_buffer);

    free(buffer);
    free(enc_buffer);
    
    return 0;
}
