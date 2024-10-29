import google.generativeai as genai
import os
import argparse

# Set your API key
key = "AIzaSyAj3z8oiHbljABcdsKRAgO05d7zcNS9Bsw"

# Configure the Generative AI with the API key
genai.configure(api_key=key)

# Create the model
generation_config = {
    "temperature": 1,
    "top_p": 0.95,
    "top_k": 64,
    "max_output_tokens": 8192,
    "response_mime_type": "text/plain",
}

model = genai.GenerativeModel(
    model_name='tunedModels/kufi-2165',
    generation_config=generation_config,
)

def generate_text(prompt):
    """Function to generate text from a prompt."""
    result = model.generate_content(prompt)
    return result.text

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate text from a prompt')
    parser.add_argument('prompt', type=str, help='The prompt for text generation')
    args = parser.parse_args()

    # Generate and print the text based on the provided prompt
    print(generate_text(args.prompt))