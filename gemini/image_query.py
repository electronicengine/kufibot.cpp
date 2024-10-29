
import google.generativeai as genai
import os
import random
import time
import cv2

key = "AIzaSyAj3z8oiHbljABcdsKRAgO05d7zcNS9Bsw"
# os_key = os.environ["GEMINI_API_KEY"]

genai.configure(api_key=key)
# Create the model

def take_photo():
    # Open the camera (0 is typically the default camera)
    cam = cv2.VideoCapture(0)

    if not cam.isOpened():
        print("Error: Could not open the camera.")
        return

    # Read a frame from the camera
    ret, frame = cam.read()
    
    if ret:
        # Save the frame as 'image.jpg' in the current directory
        cv2.imwrite("image.jpg", frame)
        print("Photo saved as 'image.jpg'")
    else:
        print("Error: Could not read frame from the camera.")

    # Release the camera resource
    cam.release()
    
    
sample_file = genai.upload_file(path="image.jpg", display_name="Sample drawing")

file = genai.get_file(name=sample_file.name)
model = genai.GenerativeModel(model_name="models/gemini-1.5-flash")

response = model.generate_content(
    ["bu resmi türkçe olarak tanımla", sample_file]
)

print(response.text)
     