import google.generativeai as genai
import argparse
import sys

API_KEY = "AIzaSyAj3z8oiHbljABcdsKRAgO05d7zcNS9Bsw"
MODEL_NAME = "models/gemini-1.5-flash"

genai.configure(api_key=API_KEY)
file = genai.get_file(name="image")

sample_file = genai.upload_file(path="image.jpg", display_name="image")
genai.delete_file(sample_file.name)

def generate_text(prompt):
    """Generate text based on the prompt and uploaded image."""
    model = genai.GenerativeModel(model_name=MODEL_NAME)
    response = model.generate_content([prompt, sample_file])

    return response.text

if __name__ == "__main__":
    # Set up argument parsing
    parser = argparse.ArgumentParser(description='Generate text from a prompt')
    parser.add_argument('prompt', type=str, help='The prompt for text generation')
    args = parser.parse_args()

    try:
        output = generate_text(args.prompt)
        print(output)  # Print the result so C++ can read it
        sys.stdout.flush()  # Ensure all output is flushed
    except Exception as e:
        print(f"Error: {str(e)}", file=sys.stderr)
        sys.exit(1)