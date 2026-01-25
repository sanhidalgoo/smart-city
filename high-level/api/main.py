import serial
import time
import json
import firebase_admin
from firebase_admin import credentials, db as firebase_db
import google.generativeai as genai
import os
from dotenv import load_dotenv

load_dotenv()

# Configura el puerto serial y la velocidad de baudios (asegúrate de que coincida con el de Arduino)
arduino_port = "/dev/ttyUSB0"  # Reemplaza con el puerto serial de tu Arduino
baud_rate = 9600

# Carga tus credenciales de Firebase Realtime Database
cred = credentials.Certificate("./service_account.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://test2-a0f86-default-rtdb.firebaseio.com/'
})

# Referencia a la Realtime Database
db = firebase_db.reference()

genai.configure(api_key=os.getenv("GEMINI_API_KEY"))
generation_config = {
  "temperature": 1,
  "top_p": 0.95,
  "top_k": 64,
  "max_output_tokens": 8192,
  "response_mime_type": "text/plain",
}
safety_settings = [
  {
    "category": "HARM_CATEGORY_HARASSMENT",
    "threshold": "BLOCK_MEDIUM_AND_ABOVE",
  },
  {
    "category": "HARM_CATEGORY_HATE_SPEECH",
    "threshold": "BLOCK_MEDIUM_AND_ABOVE",
  },
  {
    "category": "HARM_CATEGORY_SEXUALLY_EXPLICIT",
    "threshold": "BLOCK_MEDIUM_AND_ABOVE",
  },
  {
    "category": "HARM_CATEGORY_DANGEROUS_CONTENT",
    "threshold": "BLOCK_MEDIUM_AND_ABOVE",
  },
]

system_instruction = ""

with open("./agent-prompt.md", "r") as file:
    system_instruction = file.read()

model = genai.GenerativeModel(
  model_name="gemini-2.5-flash",
  safety_settings=safety_settings,
  generation_config=generation_config,
  system_instruction=system_instruction
)

chat_session = model.start_chat(
  history=[
  ]
)

def connect_serial():
    while True:
        try:
            ser = serial.Serial(arduino_port, baud_rate, timeout=1)
            print("Serial connection established.")
            return ser
        except serial.SerialException as e:
            print(f"Failed to connect to serial port '{arduino_port}': {e}")
            print("Make sure:")
            print("  1. Arduino is connected via USB")
            print("  2. The port is correct (check with: ls /dev/ttyUSB* /dev/ttyACM*)")
            print("  3. You have permission (try: sudo chmod 666 /dev/ttyUSB0)")
            print("Retrying in 10 seconds...")
            time.sleep(10)

def read_arduino(ser):
    try:
        if ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            try:
                data = json.loads(line)
                return data
            except json.JSONDecodeError:
                print(f"Failed to decode JSON: {line}")
                return None
    except serial.SerialException:
        print("Device disconnected. Attempting to reconnect...")
        time.sleep(10)  # Wait before attempting to reconnect
        ser = connect_serial()
    return None

def send_timings(timings: dict):
    timings = json.dumps(timings)
    print(f"Sending timings to Arduino: {timings.encode()}")
    ser.write(timings.encode())
    ser.flush()

def save_to_firebase(data):
    db.child("arduino_data").push(data)

try:
    ser = connect_serial()
    while True:
        try:
            sensor_data = read_arduino(ser)
            if sensor_data:
                print(f"Received from Arduino: {sensor_data}")
                print(f"Sending to Gemini: {json.dumps(sensor_data)}")
                response = chat_session.send_message(json.dumps(sensor_data)).text.replace("```json", "").replace("```", "").strip()
                response_json = json.loads(response)

                # Print the full response
                print(f"Response from Gemini: {response_json}")

                # Print the agent's reasoning
                if "reasoning" in response_json:
                    print(f"\n🤖 Agent reasoning: {response_json['reasoning']}\n")

                # Save sensor data to Firebase (disabled for now)
                # save_to_firebase(sensor_data)

                # Send timings to Arduino
                if "timings" in response_json:
                    print(f"Sending timings to Arduino: {response_json['timings']}")
                    send_timings(response_json["timings"])
            time.sleep(1)
        except Exception as e:
            print(f"An error occurred: {e}")
            time.sleep(1)
            continue

except KeyboardInterrupt:
    print("\nExiting program.")

finally:
    ser.close()  # Cierra la conexión serial