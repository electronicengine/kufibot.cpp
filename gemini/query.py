import google.generativeai as genai
from flask import Flask, request, jsonify

# Set your API key
key = "AIzaSyAk-DrQQFK-Qt7qgZfbfyM-Pq5X9O6e7Wk"

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
    # model_name='tunedModels/kufi-2165',
    model_name='gemini-1.5-flash',
    generation_config=generation_config,
)

# Initialize Flask app
app = Flask(__name__)

def generate_text(prompt):
    """Function to generate text from a prompt."""
    result = model.generate_content(prompt)
    return result.text

@app.route('/generate', methods=['POST'])
def generate():
    """Endpoint to generate text based on a prompt."""
    data = request.get_json()
    if not data or 'prompt' not in data:
        return jsonify({'error': 'Missing "prompt" in request'}), 400

    prompt = data['prompt']
    try:
        text = generate_text(prompt)
        return jsonify({'generated_text': text}), 200
    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000)