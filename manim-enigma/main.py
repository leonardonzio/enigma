from enum import Enum
from manim import *
import ctypes

class Wiring(Enum):
    """
    enum to match the rotor/reflector wirings in enigma library
    """
    ROTOR_I = "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
    ROTOR_II = "AJDKSIRUXBLHWTMCQGZNPYFVO"
    ROTOR_III = "BDFHJLCPRTXVZNYEIWGAKMUSQO"
    ROTOR_III_STEP1 = "CEGIKBOQSWUYMXDHVFZJLTRPNA"
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
        orientation: str = "tangent"  # upright, radial, tangent
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
        for i, ch in enumerate(wiring.value):
            # angle starts at top (PI/2) and goes clockwise ( -i)
            angle = (PI/2) - i * (PI*2/(len(wiring.value))) # len(wiring.value) usually is 26
            pos = np.array([np.cos(angle), np.sin(angle), 0.0]) * r_mid

            t = Text(ch, font=font, font_size=font_size, weight=BOLD)
            t.set_color(letter_color)
            t.set_stroke(BLACK, width=0.6, opacity=0.7)
            t.move_to(pos)

            # orientation isnt upright
            if orientation == "radial":
                t.rotate(angle)
            elif orientation == "tangent":
                t.rotate(angle - PI/2)

            letters.add(t)

        return VGroup(ring, highlight, letters)

    def update_content(
        self, 
        current_object, 
        new_content, 
        content_type="text", 
        **kwargs):
        """
        function to update the content of a rotor/text mobject
        """

        if content_type == "text":
            default_kwargs = {
                'font_size': 6,
                'font': "Source Code Pro"
            }
            default_kwargs.update(kwargs)
            new_object = Text(new_content, **default_kwargs)
        elif content_type == "rotor":
            new_object = self.create_rotor(new_content, **kwargs)
        else:
            raise ValueError(f"Unsupported content_type: {content_type}")
        
        new_object.move_to(current_object)
        self.play(Transform(current_object, new_object))
        return current_object

    def construct(self):
        
        # loading shared library
        try:
            so_file = "../enigma.so"
            enigma = ctypes.CDLL(so_file)
        except OSError as e:
            print(f"Error loading shared library: {e}")
            return

        # specifying arg and return types of the encrypt function
        encrypt = enigma.encrypt
        encrypt.restype = EncryptionSteps
        encrypt.argtypes = [ctypes.c_char];
       
        # calling encrypt(char c)
        print("Type a letter to encrypt (A-Z): ", end="")
        user_input = input().strip().upper()
        steps = encrypt(user_input.encode('utf-8'))
        print(steps)

        # asking if proceeding with video
        print("Animation? (y/n): ", end="")
        proceed = input().strip().lower()
        if proceed != 'y':
            print("Aborting animation..")
            return
         
        # display input character
        input_char = steps.input_char.decode('utf-8')
        t1 = Text(input_char, font_size=6, font="Source Code Pro")
        self.play(Write(t1))
        self.wait(1)
        self.play(t1.animate.shift(UP))

        # display right rotor
        rotor_r: VGroup = self.create_rotor(Wiring.ROTOR_III)
        rotor_r.shift(DOWN*0.5)
        self.update_content(t1, "Prendo il rotore III della macchina enigma nella sua posizione iniziale.")
        self.play(Create(rotor_r))
        self.wait(1)
        
        # step rotor
        self.update_content(t1, f"""
            Ora premo il tasto '{input_char}'\n
            Siccome è il primo rotore (quello più a destra), fa uno step in avanti prima che parta il segnale elettrico.
            """, "text")
        self.wait(2)
        self.update_content(t1, f"""
            Questa rotazione sposta fisicamente l'intero corpo del rotore di 1/26 di giro, cambiando i punti di contatto.\n
            Il risultato è che i cablaggi interni non sono più allineati come prima. L'ingresso {input_char} della tastiera non si connette più al pin {input_char} del rotore, ma a quello successivo, il pin TODO.
            """, "text")
        self.update_content(rotor_r, Wiring.ROTOR_III_STEP1, "rotor")
        self.wait(2)






