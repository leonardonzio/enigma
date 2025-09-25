from enum import Enum
from manim import *
import ctypes

class Wiring(Enum):
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

def steps_to_string(steps: EncryptionSteps) -> str:
    # Decode bytes to chars and create a formatted multi-line string
    return (
        f"Input char: {steps.input_char.decode('utf-8')}\n"
        f"After plugboard 1: {steps.after_plugboard_1.decode('utf-8')}\n"
        f"After R rotor: {steps.after_R_rotor.decode('utf-8')}\n"
        f"After M rotor: {steps.after_M_rotor.decode('utf-8')}\n"
        f"After L rotor: {steps.after_L_rotor.decode('utf-8')}\n"
        f"After reflector: {steps.after_reflector.decode('utf-8')}\n"
        f"After L rotor back: {steps.after_L_rotor_back.decode('utf-8')}\n"
        f"After M rotor back: {steps.after_M_rotor_back.decode('utf-8')}\n"
        f"After R rotor back: {steps.after_R_rotor_back.decode('utf-8')}\n"
        f"After plugboard 2: {steps.after_plugboard_2.decode('utf-8')}\n"
        f"Output char: {steps.output_char.decode('utf-8')}"
    )


class Enigma(Scene):
    def create_rotor(self, wiring: Wiring,
                             outer_radius: float = 0.7,
                             inner_radius: float = 0.5,
                             font: str = "Source Code Pro",
                             font_size: int = 12,
                             letter_color = WHITE,
                             orientation: str = "tangent"  # upright, radial, tangent
                             ):
        
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
            angle = (PI/2) - i * (PI*2/26)
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

    def update_text_content(self, current_text, new_content, **text_kwargs):
        """ 
        helper function to animate change of text
        """
        default_kwargs = {
            'font_size': 8,
            'font': "Source Code Pro"
        }
        default_kwargs.update(text_kwargs)
        
        new_text = Text(new_content, **default_kwargs)
        new_text.move_to(current_text)
        self.play(Transform(current_text, new_text))
        return current_text

    def construct(self):
        # get every steps the letter does
        try:
            so_file = "../enigma.so"
            enigma = ctypes.CDLL(so_file)
        except OSError as e:
            self.add(Text(f"Error loading {so_file}: {e}", font_size=10))
            self.wait(2)
            return

        encrypt_A = enigma.encrypt_A
        encrypt_A.restype = EncryptionSteps

        steps = encrypt_A()

        steps_str = steps_to_string(steps)
        steps_text = Text(steps_str, font_size=8, font="Source Code Pro")
        self.add(steps_text)

        # display input character
        input_char = steps.input_char.decode('utf-8')
        t1 = Text(input_char, font_size=8, font="Source Code Pro")
        self.play(Write(t1))
        self.wait(1)
        self.play(t1.animate.shift(UP))

        # display right rotor
        rotor_r: VGroup = self.create_rotor(Wiring.ROTOR_III)
        rotor_r.shift(DOWN*0.5)
        self.play(Create(rotor_r))
        self.wait(1)
    
        # update text
        self.update_text_content(t1,"'A' si trova all'indice alfabetico corrispondente allo '0'.\n(A=0, B=1, ..., Z=25)")
        self.wait(2)
        

        self.update_text_content(t1, "Quindi mi sposto alla posizione 0 del rotore, dove in questo caso è presente la lettera E.")
        self.wait(1)






