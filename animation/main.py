from enum import Enum
from manim import *
import ctypes

def character_alphabet_index(char: str):
    return Wiring.ALPHABET.value.index(char)

def alphabet_character_at_index(index: int):
    return Wiring.ALPHABET.value[index]

class Wiring(Enum):
    """
    enum to match the rotor/reflector wirings in enigma library
    """
    ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    ROTOR_I = "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
    ROTOR_II = "AJDKSIRUXBLHWTMCQGZNPYFVO"
    ROTOR_III = "BDFHJLCPRTXVZNYEIWGAKMUSQO"
    REFLECTOR_B = "YRUHQSLDPXNGOKMIEBFZCWVJAT"


class EncryptionSteps(ctypes.Structure):
    """
    struct to match EncryptionSteps in enigma.c
    """
    _fields_ = [
        ('input_char', ctypes.c_char),
        ('after_plugboard_1', ctypes.c_char),
        ('after_R_rotor', ctypes.c_char),
        ('after_M_rotor', ctypes.c_char),
        ('after_L_rotor', ctypes.c_char),
        ('after_reflector', ctypes.c_char),
        ('after_L_rotor_back', ctypes.c_char),
        ('after_M_rotor_back', ctypes.c_char),
        ('after_R_rotor_back', ctypes.c_char),
        ('after_plugboard_2', ctypes.c_char),
        ('output_char', ctypes.c_char)
    ]
    
    def __str__(self) -> str:
        return (
            f"Input char:           {self.input_char.decode('utf-8')}\n"
            f"After plugboard 1:    {self.after_plugboard_1.decode('utf-8')}\n"
            f"After R rotor:        {self.after_R_rotor.decode('utf-8')}\n"
            f"After M rotor:        {self.after_M_rotor.decode('utf-8')}\n"
            f"After L rotor:        {self.after_L_rotor.decode('utf-8')}\n"
            f"After reflector:      {self.after_reflector.decode('utf-8')}\n"
            f"After L rotor back:   {self.after_L_rotor_back.decode('utf-8')}\n"
            f"After M rotor back:   {self.after_M_rotor_back.decode('utf-8')}\n"
            f"After R rotor back:   {self.after_R_rotor_back.decode('utf-8')}\n"
            f"After plugboard 2:    {self.after_plugboard_2.decode('utf-8')}\n"
            f"Output char:          {self.output_char.decode('utf-8')}"
        )

class Enigma(Scene):
    def create_rotor(
        self, 
        wiring: Wiring,
        outer_radius: float = 0.7,
        inner_radius: float = 0.5,
        font: str = "Source Code Pro",
        font_size: int = 12,
        letter_color:ManimColor = WHITE,
        orientation: str = "upright"  # upright, radial, tangent
        ) -> VGroup:
        
        # Base annulus (the rotor face as a ring)
        ring = Annulus(inner_radius=inner_radius, outer_radius=outer_radius)
        ring.set_fill(GRAY_E, opacity=1.0).set_stroke(GRAY_D, width=2)

        # Subtle highlight band for depth
        highlight = Annulus(inner_radius=(inner_radius+outer_radius)/2 * 0.95,
                            outer_radius=(inner_radius+outer_radius)/2 * 1.05)
        highlight.set_fill(GRAY_C, opacity=0.25).set_stroke(width=0)

        # Letters placed along the midline of the annulus
        letters = VGroup()
        r_mid = 0.5 * (inner_radius + outer_radius)
        n = len(wiring.value) # usually 26 ?
        
        for i, ch in enumerate(wiring.value):
            # angle starts at top (PI/2) and goes clockwise ( -i)
            angle = (PI/2) - i * (PI*2/n) 
            pos = np.array([np.cos(angle), np.sin(angle), 0.0]) * r_mid

            t = Text(ch, font=font, font_size=font_size, weight=BOLD)
            t.set_color(letter_color)
            t.set_stroke(BLACK, width=0.6, opacity=0.7)
           
            """
            idx = Text(str(i), font=font, font_size=4)
            idx.set_color(letter_color).set_opacity(0.8)
            idx.set_stroke(BLACK, width=0.4, opacity=0.6)
            idx.next_to(t,RIGHT, buff=0.03)

            pair = VGroup(t, idx)
            """

            # orientation isnt upright
            if orientation == "radial":
                t.rotate(angle)
            elif orientation == "tangent":
                t.rotate(angle - PI/2)

            t.move_to(pos)

            letters.add(t)

        return VGroup(ring, highlight, letters)

    def update_content(
        self,
        current_object,
        new_content,
        content_type="text",
        ):
        """
        function to update the content of a rotor/text object
        """

        if content_type == "text":
            new_object = Text(new_content, font_size=8, font="Source Code Pro")
        elif content_type == "rotor":
            new_object = self.create_rotor(new_content)
        else:
            raise ValueError(f"content_type inesistente: {content_type}")

        new_object.move_to(current_object)
        self.play(Transform(current_object, new_object))
        return current_object

    def construct(self):
        # loading shared library
        try:
            so_file = "build/enigma.so"
            enigma = ctypes.CDLL(so_file)
        except OSError as e:
            print(f"Error loading shared library: {e}")
            return

        # specifying arg and return types of the trace_encrypt function
        trace_encrypt = enigma.trace_encrypt;
        trace_encrypt.restype = EncryptionSteps
        trace_encrypt.argtypes = [ctypes.c_char];

        # calling encrypt(char c)
        print("Type a letter to encrypt (A-Z): ", end="")
        user_input = input().strip().upper()
        steps = trace_encrypt(user_input.encode('utf-8'))
        print(steps)

        # asking if proceeding with video
        print("Animation? (y/n): ", end="")
        proceed = input().strip().lower()
        if proceed != 'y':
            print("Aborting animation..")
            return

        input_char = steps.input_char.decode('utf-8')
        after_R_rotor = steps.after_R_rotor.decode('utf-8')

        # display text
        t1 = Text(f"""Prendo il rotore III ({Wiring.ROTOR_III.value}) della macchina enigma\n
            nella sua posizione iniziale (parto dall'indice 0).
            """,
            font_size=8, font="Source Code Pro")
        t1.shift(UP)
        self.play(Write(t1))
        self.wait(1)

        # display right rotor
        rotor_r: VGroup = self.create_rotor(Wiring.ROTOR_III)
        rotor_r.shift(DOWN*0.5)
        self.play(FadeIn(rotor_r))
        self.wait(1)

        # display pressing letter, and stepping mechanism
        self.update_content(t1, f"""
            Ora premo il tasto '{input_char}'\n
            Il rotore, essendo il primo, avanza sempre di una posizione, prima che il segnale elettrico parta.\n
            """, "text")
        self.wait(1) 
        
        input_char_t = Text(input_char, font_size=12, font="Source Code Pro")
        input_char_t.set_color_by_gradient(RED, ORANGE)
        input_char_t.shift(RIGHT)
        self.play(Write(input_char_t))
        
        # displayng what happens after stepping
        idx_input = character_alphabet_index(input_char)
        idx_input_stepped = (idx_input + 1) % 26
        char_at_stepped = alphabet_character_at_index(idx_input_stepped)
        self.update_content(t1, f"""
            Questa rotazione sposta fisicamente l'intero corpo del rotore di 1/26 di giro,\n
            Il risultato è che i cablaggi interni non sono più allineati come prima:\n
            L'ingresso '{input_char}' non si connette più al pin '{input_char}'({idx_input}) del rotore,\n
            ma a quello successivo (modulo 26), il pin '{char_at_stepped}'({idx_input_stepped}).\n
            """, "text")
        self.wait(1)
        
        self.update_content(t1, f"""
            Quindi, entra il segnale della lettera '{input_char}' (indice {idx_input}),\n
            ma a causa dello step (+1), il segnale segue il percorso del cablaggio che parte dal pin all'indice {idx_input_stepped} ({idx_input} + 1 modulo 26).
            """, "text")
        self.wait(1)

        self.update_content(t1, f"""
            Il cablaggio del Rotore III al nuovo indice {idx_input_stepped} ha la lettera {Wiring.ROTOR_III.value[idx_input_stepped]}.\n
        """, "text")
        self.wait(1)
        
        letter = rotor_r[2][idx_input_stepped].set_color(YELLOW) # rotor_r[2]: letters in VGroup
        self.play(letter.animate.set_color(YELLOW))
        self.wait(1)
        self.play(letter.animate.set_color(WHITE))
        self.wait(1)

        self.update_content(t1, f"""
            Tuttavia anche l'uscita è sfalsata.\n
            Il segnale esce dal contatto {Wiring.ROTOR_III.value[idx_input_stepped]} (indice {character_alphabet_index(Wiring.ROTOR_III.value[idx_input_stepped])}), ma siccome tutto il rotore è ruotato, dobbiamo sottrarre l'offset: {character_alphabet_index(Wiring.ROTOR_III.value[idx_input_stepped])} - 1 = {character_alphabet_index(after_R_rotor)}.
            """, "text")
        self.wait(1)

        self.update_content(t1, f"""
            L'indice {character_alphabet_index(after_R_rotor)} corrisponde alla lettera '{after_R_rotor}'.
            Quindi, a causa di un singolo step, la lettera '{input_char}' è stata trasformata in '{after_R_rotor}'\n
            invece che in '{Wiring.ROTOR_III.value[idx_input_stepped]}'. \n
            Il segnale '{after_R_rotor}' ora prosegue verso il rotore successivo.
            """, "text")
        self.wait(1)

        letter_rotor = rotor_r[2][character_alphabet_index(after_R_rotor)]
        self.play(letter_rotor.animate.set_color(YELLOW))
        self.wait(1)
        
        self.play(Transform(input_char_t, letter_rotor))
        self.wait(1)
        
        """
        MIDDLE ROTOR
        """
        # saving new letter
        self.update_content(t1, f"""
            Stessa cosa ora succede per i rotori successivi, ma senza lo step in avanti.\n 
            """, "text")
        self.wait(1)
        self.update_content(t1, f"""
            Prendo il rotore di mezzo (il rotore II):
            """, "text")

        # removing old rotor and creating new middle rotor
        self.remove(rotor_r, input_char_t)
        after_R_rotor_t = Text(after_R_rotor, font_size=12, font="Source Code Pro")
        after_R_rotor_t.set_color_by_gradient(RED, ORANGE)
        after_R_rotor_t.shift(RIGHT)
        self.play(FadeIn(after_R_rotor_t))

        rotor_m = self.create_rotor(Wiring.ROTOR_II)
        rotor_m.shift(DOWN*0.5)
        self.play(FadeIn(rotor_m))
        self.wait(1)
        
        # letter enters middle rotor
        after_M_rotor = steps.after_M_rotor.decode('utf-8')
        self.update_content(t1, f"""
            La lettera {after_R_rotor} ha indice alfabetico {character_alphabet_index(after_R_rotor)},\n
            entra dunque alla posizione di indice {character_alphabet_index(after_R_rotor)} del rotore,\n
            che equivale alla lettera {after_M_rotor} sul rotore.
            """, "text")
        self.wait(1)

        letter_rotor = rotor_m[2][character_alphabet_index(after_R_rotor)] # rotor_r[2]: letters in VGroup
        self.play(letter_rotor.animate.set_color(YELLOW))
        self.wait(1)
        
        self.play(Transform(after_R_rotor_t, letter_rotor))
        self.wait(1)


        """
        LEFT ROTOR
        """
        # saving new letter
        self.update_content(t1, f"""
            Prendo il rotore più a sinistra (il rotore I):
            """, "text")
       
        # removing old rotor and creating new left rotor
        self.remove(rotor_m,after_R_rotor_t)
        after_M_rotor_t = Text(after_M_rotor, font_size=12, font="Source Code Pro")
        after_M_rotor_t.set_color_by_gradient(RED, ORANGE)
        after_M_rotor_t.shift(RIGHT)
        self.play(FadeIn(after_M_rotor_t))

        rotor_l = self.create_rotor(Wiring.ROTOR_I)
        rotor_l.shift(DOWN*0.5)
        self.play(FadeIn(rotor_l))
        self.wait(1)
        
        # letter enters left rotor
        after_L_rotor = steps.after_L_rotor.decode('utf-8')
        self.update_content(t1, f"""
            La lettera {after_M_rotor} ha indice alfabetico {character_alphabet_index(after_M_rotor)},\n
            entra dunque alla posizione di indice {character_alphabet_index(after_M_rotor)} del rotore,\n
            che equivale alla lettera {after_L_rotor} sul rotore.
            """, "text")
        self.wait(1)

        letter_rotor = rotor_l[2][character_alphabet_index(after_M_rotor)] # rotor_r[2]: letters in VGroup
        self.play(letter_rotor.animate.set_color(YELLOW))
        self.wait(1)
        
        self.play(Transform(after_M_rotor_t, letter_rotor))
        self.wait(1)


        """
        REFLECTOR B
        """
        # saving new letter
        self.update_content(t1, f"""
            Ora sono finiti i rotori, e giungiamo a un riflettore, in questo caso il riflettore 'B': 
            """, "text")
       
        # removing old rotor and creating reflector
        self.remove(rotor_l,after_M_rotor_t)
        after_L_rotor_t = Text(after_L_rotor, font_size=12, font="Source Code Pro")
        after_L_rotor_t.set_color_by_gradient(RED, ORANGE)
        after_L_rotor_t.shift(RIGHT)
        self.play(FadeIn(after_L_rotor_t))

        reflector = self.create_rotor(Wiring.REFLECTOR_B)
        reflector.shift(DOWN*0.5)
        self.play(FadeIn(reflector))
        self.wait(1)
        
        # letter enters Middle rotor
        after_reflector = steps.after_reflector.decode('utf-8')
        self.update_content(t1, f"""
            Il riflettore ha un compito semplice: invertire il percorso del segnale.
            1 -> La lettera '{after_L_rotor}' (indice {character_alphabet_index(after_L_rotor)}) entra e colpisce il pin corrispondente.
            2 -> Quel pin è cablato direttamente alla lettera '{after_reflector}'.
            3 -> Il segnale "rimbalza" e torna indietro nei rotori, partendo dal contatto corrispondente a '{after_reflector}'.
            """, "text")
        self.wait(1)

        reflector_letter = reflector[2][character_alphabet_index(after_L_rotor)] # rotor_r[2]: letters in VGroup
        self.play(reflector_letter.animate.set_color(YELLOW))
        self.wait(1)

        self.play(Transform(after_L_rotor_t, reflector_letter))
        self.wait(1)
