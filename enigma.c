/*
 * Enigma M3 Simulator
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* --------- constants + macros */
#define ALPHABET_SIZE 26
#define NUM_ROTORS 3

#define INDEX_TO_C(index)   ((char) ('A' + (index)))    // da numero dell alfabeto al carattere corrispondente
#define C_TO_INDEX(c)       ((int)  ((c) - 'A'))        // da carattere al numero dell alfabeto

/* --------- structs & vars */
typedef char Wiring[ALPHABET_SIZE];

typedef struct {
    Wiring wiring;
    const char *name;
} Reflector;

typedef struct {
    Wiring wiring;
    char notch;
    int position;
    int ring_setting;
    const char *name;
} Rotor;

typedef struct {
    char wiring[ALPHABET_SIZE]; 
} Plugboard;

enum { RIGHT = 0, MIDDLE = 1, LEFT = 2 };

// https://www.ciphermachinesandcryptology.com/en/enigmatech.htm
static Reflector ALL_REFLECTORS[] = {
    { .wiring = "YRUHQSLDPXNGOKMIEBFZCWVJAT", .name = "Reflector B" },
    { .wiring = "FVPJIAOYEDRZXWGCTKUQSBNMHL", .name = "Reflector C" }
};

// https://www.codesandciphers.org.uk/enigma/rotorspec.htm
static Rotor ALL_ROTORS[] = {
    /* a ring setting of 0 its the same as A (A0, B1, .. , Z25) */
    /* position can be A0, B1, .. , Z25 */
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .notch = 'Q', .position = 0, .ring_setting = 0, .name = "Rotor I" },
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .notch = 'E', .position = 0, .ring_setting = 0, .name = "Rotor II" },
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .notch = 'V', .position = 0, .ring_setting = 0, .name = "Rotor III" },
    { .wiring = "ESOVPZJAYQUIRHXLNFTGKDCMWB", .notch = 'J', .position = 0, .ring_setting = 0, .name = "Rotor IV"},
    { .wiring = "VZBRGITYUPSDNHLXAWMJQOFECK", .notch = 'Z', .position = 0, .ring_setting = 0, .name = "Rotor V"}
    // { .wiring = "JPGVOUMFYQBENHZRDKASXLICTW", .notch = '', .position = 0, .name = "Rotor VI"}
    // { .wiring = "NZJHGRCXMYSWBOUFAIVLPEKQDT", .notch = '', .position = 0, .name = "Rotor VII"}
    // { .wiring = "FKQHTLXOCBJSPDZRAMEWNIUYGV", .notch = '', .position = 0, .name = "Rotor VIII"}
};

static Plugboard PLUGBOARD_CONFIGS[] = {
    { .wiring = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, // no connections
    { .wiring = "ABQDEFGHIJKLMNOPCRSTUVWXYZ" }  // Q swapped with C
};


/* --------- functions */
static inline int mod26(int x) {
    return (x % 26 + 26) % 26;
}

static char rotor_forward(char c, const Rotor *r) {
    int index = mod26(C_TO_INDEX(c) + r->position - r->ring_setting);
    char wired = r->wiring[index];
    return INDEX_TO_C(mod26(C_TO_INDEX(wired) - r->position + r->ring_setting));
}

static char rotor_backward(char c, const Rotor *r) {
    int shifted_c = mod26(C_TO_INDEX(c) + r->position - r->ring_setting);
    int inverse_index = 0;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (r->wiring[i] == INDEX_TO_C(shifted_c)) {
            inverse_index = i;
            break;
        }
    }
    return INDEX_TO_C(mod26(inverse_index - r->position + r->ring_setting));
}

static void print_status(Rotor rotors[]){
    int i, j;
    printf("\n=====STATUS=====\n");
    for (i = 0; i < NUM_ROTORS; i++){

        printf("Name: %s:\n", rotors[i].name);
        printf("Wiring: ");
        for(j = 0; j < ALPHABET_SIZE; j++){
            printf("%c", rotors[i].wiring[j]);
        }
        printf("\nPosition: %d (%c)\n", rotors[i].position, INDEX_TO_C(rotors[i].position));
        printf("Ring setting: %d (%c)\n", rotors[i].ring_setting, INDEX_TO_C(rotors[i].ring_setting));
        printf("Notch: %c\n", rotors[i].notch);
        printf("--------------------\n");
    }
    printf("=======END=======\n\n");
}

char encrypt_char(char c, Rotor rotors[], Reflector reflector) {
    // forward path
    for (int i = 0; i < NUM_ROTORS; i++) {
        c = rotor_forward(c, &rotors[i]);
    }

    // reflector
    c = reflector.wiring[C_TO_INDEX(c)];

    // backward path
    for (int i = NUM_ROTORS - 1; i >= 0; i--) {
        c = rotor_backward(c, &rotors[i]);
    }

    return c;
}

void step_rotors(Rotor r[]){
    uint8_t right_at_notch  = (mod26(r[RIGHT].position - r[RIGHT].ring_setting) == C_TO_INDEX(r[RIGHT].notch));
    uint8_t middle_at_notch = (mod26(r[MIDDLE].position - r[MIDDLE].ring_setting) == C_TO_INDEX(r[MIDDLE].notch));

    // double step ( se middle su notch, avanza anche left)
    if (middle_at_notch) {
        r[LEFT].position = (r[LEFT].position + 1) % ALPHABET_SIZE;
    }

    // middle avanza se right su notch || middle su notch
    if (right_at_notch || middle_at_notch) {
        r[MIDDLE].position = (r[MIDDLE].position + 1) % ALPHABET_SIZE;
    }

    // right avanza sempre
    r[RIGHT].position = (r[RIGHT].position + 1) % ALPHABET_SIZE;
}

char enter_plugboard(char c, Plugboard plugboard){
    for (int i = 0; i < ALPHABET_SIZE; i++){
        if (c == plugboard.wiring[i]) return INDEX_TO_C(i);
    }
    return c;
}

void choose_rotors(Rotor rotors[]) {
    int choice;
    printf("Available rotors:\n");
    for (size_t i = 0; i < NUM_ROTORS; i++) {
        printf("%zu: %s\n", i + 1, ALL_ROTORS[i].name);
    }
    printf("\n");
     
    printf("Choose the right rotor (1-%d): ", NUM_ROTORS);
    scanf("%d", &choice);
    rotors[RIGHT] = ALL_ROTORS[choice - 1];

    printf("Choose the middle rotor (1-%d): ", NUM_ROTORS);
    scanf("%d", &choice);
    rotors[MIDDLE] = ALL_ROTORS[choice - 1]; 

    printf("Choose the left rotor (1-%d): ", NUM_ROTORS);
    scanf("%d", &choice);
    rotors[LEFT] = ALL_ROTORS[choice - 1]; 

    while (getchar() != '\n'); 
}


/* needed for thesis */
char encrypt_A() {
    
    Reflector reflector = ALL_REFLECTORS[0]; // Reflector B
    Rotor rotors[NUM_ROTORS];
    rotors[RIGHT]  = ALL_ROTORS[2]; // Rotor III
    rotors[MIDDLE] = ALL_ROTORS[1]; // Rotor II
    rotors[LEFT]   = ALL_ROTORS[0]; // Rotor I

    char c = 'A';

    step_rotors(rotors);
    
    c = enter_plugboard(c, PLUGBOARD_CONFIGS[0]); // nessun collegamento
    c = encrypt_char(c, rotors, reflector);
    c = enter_plugboard(c, PLUGBOARD_CONFIGS[0]); // nessun collegamento

    return c;
}


int main(int argc, char *argv[]) {
    
    printf(" _____       _                             __  __ _____\n");
    printf("| ____|_ __ (_) __ _ _ __ ___   __ _      |  \\/  |___ /\n");
    printf("|  _| | '_ \\| |/ _` | '_ ` _ \\ / _` |_____| |\\/| | |_ \\\n");
    printf("| |___| | | | | (_| | | | | | | (_| |_____| |  | |___) |\n");
    printf("|_____|_| |_|_|\\__, |_| |_| |_|\\__,_|     |_|  |_|____/\n");
    printf("               |___/                                   \n");
    
    Reflector reflector;
    Rotor rotors[NUM_ROTORS];
    
    // choose reflector
    printf("Choose the reflector (B or C): ");
    int ref_choice = getchar();
    getchar();
    if (ref_choice == 'B'){
        reflector = ALL_REFLECTORS[0];
    } else if (ref_choice == 'C'){
        reflector = ALL_REFLECTORS[1];
    } else {
        printf("Invalid choice. Deafult is Reflector B.\n");
        reflector = ALL_REFLECTORS[0];
    }
    printf("Selected %s.\n", reflector.name);

    choose_rotors(rotors);

    print_status(rotors);
 
    printf("Enter word to encrypt: (only uppercase letters, no spaces)\n");

    char line[1024];
    if ( ! fgets(line, sizeof(line), stdin)){
        perror("error reading input string to encrypt");
        return 1;
    }
    
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
        len--;
    }

    // alloco buffer per salvare stringa cifrata
    char *enc_buffer = malloc(len + 1);
    if ( ! enc_buffer) {
        perror("error allocating memory for encrypted buffer");
        return 1;
    }
    
    for (size_t i = 0; line[i] != '\0'; i++) {

        printf("encrypting character: %c\n", line[i]);
    
        // step rotors
        printf("Stepping rotors...\n");
        step_rotors(rotors);
        print_status(rotors);

        // enter plugboard
        char c = enter_plugboard(line[i], PLUGBOARD_CONFIGS[0]); // nessun collegamento
        printf("Character after plugboard (in): %c\n", c);

        // rotors -> reflector -> rotors
        char enc_char = encrypt_char(c, rotors, reflector);

        // enter plugboard
        enc_char = enter_plugboard(enc_char, PLUGBOARD_CONFIGS[0]);
        printf("encrypted character: %c -> %c\n", c, enc_char);

        enc_buffer[i] = enc_char;
    }

    enc_buffer[len] = '\0';
    printf("\nEncrypted word: %s\n", enc_buffer);
    printf("if i encrypt A, output should be: %c\n", encrypt_A());
    free(enc_buffer);
    return 0;
}
