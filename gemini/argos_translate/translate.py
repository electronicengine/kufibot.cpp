import argostranslate.package
import argostranslate.translate
import os
import sys
import warnings

# Redirecting stderr to devnull to suppress error messages
sys.stderr = open(os.devnull, 'w')
warnings.simplefilter("ignore", category=FutureWarning)

from_code = "tr"
to_code = "en"

while True:
    user_input = input("Enter something: ")
    # Translate
    translatedText = argostranslate.translate.translate(user_input, from_code, to_code)
    print(translatedText)
    # '¡Hola Mundo!'