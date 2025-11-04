/* Enigma M3 Simulator */

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* --------- constants + macros */
#define ALPHABET_SIZE 26
#define NUM_ROTORS 3

#define INDEX_TO_C(index) ((char) ('A' + (index)))
#define C_TO_INDEX(c)     ((int)  ((c) - 'A'))

static int 
mod26 (int x)
{
    return (x % 26 + 26) % 26;
}

/* rotor direction */
enum { RIGHT = 0, MIDDLE = 1, LEFT = 2 };

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

/* struct to keep track of each step during encryption, for manim animation */
typedef struct {
    char input_char;
    char after_plugboard_1;
    char after_R_rotor;
    char after_M_rotor; 
    char after_L_rotor;
    char after_reflector;
    char after_L_rotor_back;
    char after_M_rotor_back; 
    char after_R_rotor_back;
    char after_plugboard_2;
    char output_char;
} EncryptionSteps;

typedef struct {
    Rotor rotors[NUM_ROTORS];
    Reflector reflector;
    Plugboard plugboard;
} Enigma;


// https://www.ciphermachinesandcryptology.com/en/enigmatech.htm
static Reflector ALL_REFLECTORS[] = {
    { .wiring = "YRUHQSLDPXNGOKMIEBFZCWVJAT", .name = "Reflector B" },
    { .wiring = "FVPJIAOYEDRZXWGCTKUQSBNMHL", .name = "Reflector C" }
};

// https://www.codesandciphers.org.uk/enigma/rotorspec.htm
static Rotor ALL_ROTORS[] = {
    /* 
     * a ring setting of 0 equals to A (A0, B1, .. , Z25)
     * position can be A0, B1, .. , Z25 
    */
    { .wiring = "EKMFLGDQVZNTOWYHXUSPAIBRCJ", .notch = 'Q', .position = 0, .ring_setting = 0, .name = "Rotor I" },
    { .wiring = "AJDKSIRUXBLHWTMCQGZNPYFVOE", .notch = 'E', .position = 0, .ring_setting = 0, .name = "Rotor II" },
    { .wiring = "BDFHJLCPRTXVZNYEIWGAKMUSQO", .notch = 'V', .position = 0, .ring_setting = 0, .name = "Rotor III" },
    { .wiring = "ESOVPZJAYQUIRHXLNFTGKDCMWB", .notch = 'J', .position = 0, .ring_setting = 0, .name = "Rotor IV"},
    { .wiring = "VZBRGITYUPSDNHLXAWMJQOFECK", .notch = 'Z', .position = 0, .ring_setting = 0, .name = "Rotor V"}
    /*{ .wiring = "JPGVOUMFYQBENHZRDKASXLICTW", .notch = '', .position = 0, .name = "Rotor VI"}*/
    /*{ .wiring = "NZJHGRCXMYSWBOUFAIVLPEKQDT", .notch = '', .position = 0, .name = "Rotor VII"}*/
    /*{ .wiring = "FKQHTLXOCBJSPDZRAMEWNIUYGV", .notch = '', .position = 0, .name = "Rotor VIII"}*/
};

static Plugboard PLUGBOARD_CONFIGS[] = {
    { .wiring = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" }, // no connections
    { .wiring = "ABQDEFGHIJKLMNOPCRSTUVWXYZ" }  // Q swapped with C
};


/* --------- functions */

static char 
rotor_forward (char c, const Rotor *r)
{
    int index = mod26(C_TO_INDEX(c) + r->position - r->ring_setting);
    char wired = r->wiring[index];
    return INDEX_TO_C(mod26(C_TO_INDEX(wired) - r->position + r->ring_setting));
}

static char 
rotor_backward (char c, const Rotor *r) 
{
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

static void 
print_status (Rotor rotors[])
{
    int i, j;
    printf("\n=====STATUS=====\n");
    for (i = 0; i < NUM_ROTORS; i++){
        printf("Name: %s:\n", rotors[i].name);
        printf("Wiring: ");
        for(j = 0; j < ALPHABET_SIZE; j++)
            printf("%c", rotors[i].wiring[j]);
        
        printf("\nPosition: %d (%c)\n", rotors[i].position, INDEX_TO_C(rotors[i].position));
        printf("Ring setting: %d (%c)\n", rotors[i].ring_setting, INDEX_TO_C(rotors[i].ring_setting));
        printf("Notch: %c\n", rotors[i].notch);
        printf("--------------------\n");
    }
    printf("=======END=======\n\n");
}

static char 
encrypt_character (char c, Rotor rotors[], Reflector reflector)
{
    for (int i = 0; i < NUM_ROTORS; i++){
        c = rotor_forward(c, &rotors[i]);
    } 
    
    c = reflector.wiring[C_TO_INDEX(c)];
    
    for (int i = NUM_ROTORS - 1; i >= 0; i--){
        c = rotor_backward(c, &rotors[i]);
    }

    return c;
}

static void 
step_rotors (Rotor r[])
{
    uint8_t right_at_notch  = (mod26(r[RIGHT].position - r[RIGHT].ring_setting) == C_TO_INDEX(r[RIGHT].notch));
    uint8_t middle_at_notch = (mod26(r[MIDDLE].position - r[MIDDLE].ring_setting) == C_TO_INDEX(r[MIDDLE].notch));

    // double step ( se middle su notch, avanza anche left)
    if (middle_at_notch)
        r[LEFT].position = (r[LEFT].position + 1) % ALPHABET_SIZE;

    // middle avanza se right su notch || middle su notch
    if (right_at_notch || middle_at_notch)
        r[MIDDLE].position = (r[MIDDLE].position + 1) % ALPHABET_SIZE;

    // right avanza sempre
    r[RIGHT].position = (r[RIGHT].position + 1) % ALPHABET_SIZE;
}

static char 
enter_plugboard (char c, Plugboard plugboard)
{
    for (int i = 0; i < ALPHABET_SIZE; i++){
        if (c == plugboard.wiring[i])
            return INDEX_TO_C(i);
    }
    return c;
}

static void 
choose_rotors (Rotor rotors[]) 
{
    int choice;
    printf("Available rotors:\n");
    
    for (size_t i = 0; i < NUM_ROTORS; i++){
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


EncryptionSteps 
encrypt (char c) 
{
    EncryptionSteps steps;
    Rotor rotors[NUM_ROTORS];
    Reflector reflector; 
    Plugboard plugboard;
    
    rotors[RIGHT]   = ALL_ROTORS[2];        // Rotor III
    rotors[MIDDLE]  = ALL_ROTORS[1];        // Rotor II  
    rotors[LEFT]    = ALL_ROTORS[0];        // Rotor I
    reflector       = ALL_REFLECTORS[0];    // Reflector B
    plugboard       = PLUGBOARD_CONFIGS[0]; // no connections 
    
    steps.input_char = c;
   
    /* before encrypting a char a step is needed */ 
    step_rotors(rotors);
    
    c = enter_plugboard(c, plugboard);
    steps.after_plugboard_1 = c;
    
    c = rotor_forward(c, &rotors[RIGHT]);
    steps.after_R_rotor = c;
    
    c = rotor_forward(c, &rotors[MIDDLE]);
    steps.after_M_rotor = c;
    
    c = rotor_forward(c, &rotors[LEFT]);
    steps.after_L_rotor = c;
    
    c = reflector.wiring[C_TO_INDEX(c)];
    steps.after_reflector = c;
    
    c = rotor_backward(c, &rotors[LEFT]);
    steps.after_L_rotor_back= c;
    
    c = rotor_backward(c, &rotors[MIDDLE]);
    steps.after_M_rotor_back= c;
    
    c = rotor_backward(c, &rotors[RIGHT]);
    steps.after_R_rotor_back= c;
    
    c = enter_plugboard(c, plugboard);
    steps.after_plugboard_2 = c;
    
    steps.output_char = c;
    
    return steps;
}


void 
encrypt_word (Rotor *rotors, const Reflector reflector, const char *word, char *encrypted_word)
{
    size_t i;
	for (i = 0; word[i] != '\0'; i++) {
		printf("encrypting character: %c\n", word[i]);

        /* step rotors */
        printf("Stepping rotors...\n");
        step_rotors(rotors);
        print_status(rotors);
        
        /* enter the plugboard */
        char c = enter_plugboard(word[i], PLUGBOARD_CONFIGS[0]);
        printf("Character after plugboard (in): %c\n", c);
        
        /* go thru rotors, reflector and rotors */
        char encrypted_char = encrypt_character(c, rotors, reflector);
        
        /* enter the plugboard */
        encrypted_char = enter_plugboard(encrypted_char, PLUGBOARD_CONFIGS[0]);
        printf("encrypted character: %c -> %c\n", c, encrypted_char);

        encrypted_word[i] = encrypted_char;
	}

    encrypted_word[i] = '\0';
}


int 
main (void) 
{
    printf(" _____       _                             __  __ _____\n");
    printf("| ____|_ __ (_) __ _ _ __ ___   __ _      |  \\/  |___ /\n");
    printf("|  _| | '_ \\| |/ _` | '_ ` _ \\ / _` |_____| |\\/| | |_ \\\n");
    printf("| |___| | | | | (_| | | | | | | (_| |_____| |  | |___) |\n");
    printf("|_____|_| |_|_|\\__, |_| |_| |_|\\__,_|     |_|  |_|____/\n");
    printf("               |___/                                   \n");
    
    Reflector reflector;
    Rotor rotors[NUM_ROTORS];
    
    /* choosing the reflector */
    printf("Choose the reflector (B or C): ");
    int choice = getchar();
    getchar();
    if (choice == 'B' || choice == 'b'){
        reflector = ALL_REFLECTORS[0];
    } else if (choice == 'C' || choice == 'c'){
        reflector = ALL_REFLECTORS[1];
    } else {
        printf("Invalid choice. Deafult is Reflector B.\n");
        reflector = ALL_REFLECTORS[0];
    }
    printf("Selected %s.\n", reflector.name);

    choose_rotors(rotors);
    print_status(rotors);
 
    printf("Enter word to encrypt: (only uppercase letters, no spaces)\n");
    char word[1024];
    if (! fgets(word, sizeof(word), stdin)){
        perror("error reading input string to encrypt");
        return 1;
    }
    
    /* removing '\n' from word */
    size_t len = strlen(word);
    if (len > 0 && word[len - 1] == '\n') {
        word[len - 1] = '\0';
        len--;
    }

    /* allocating buffer for encrypted string */
    char *encrypted_word = malloc(len + 1);
    if (! encrypted_word) {
        perror("error allocating memory for encrypted buffer");
        return 1;
    }
    
    encrypt_word(rotors, reflector, word, encrypted_word);
    printf("\nEncrypted word: %s\n", encrypted_word);
    
    free(encrypted_word);
    return 0;
}
