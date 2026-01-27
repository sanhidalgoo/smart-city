# Smart Traffic Controller API

Python server that bridges the Arduino traffic controller with Google Gemini AI for intelligent timing optimization.

## Overview

This API:
- Receives sensor data from Arduino via serial communication
- Sends data to Gemini AI for analysis and timing recommendations
- Forwards optimized timings back to Arduino
- Stores all data in Firebase Realtime Database

## Architecture

```
Arduino (ESP32) <--Serial--> Python API <--HTTP--> Gemini AI
                                  |
                                  v
                          Firebase RTDB
```

## Prerequisites

- Python 3.13+
- [uv](https://docs.astral.sh/uv/) package manager
- Google Gemini API key
- Firebase service account credentials
- Arduino connected via USB

## Installation

1. Navigate to the api directory:
   ```bash
   cd high-level/api
   ```

2. Install dependencies:
   ```bash
   uv sync
   ```

3. Create a `.env` file with your Gemini API key:
   ```
   GEMINI_API_KEY=your_api_key_here
   ```

4. Ensure `service_account.json` is present (Firebase credentials)

## Configuration

### Serial Port

Edit `main.py` to match your Arduino port:
```python
arduino_port = "/dev/ttyUSB0"  # Linux
# arduino_port = "/dev/ttyACM0"  # Linux (alternative)
# arduino_port = "COM3"  # Windows
```

Find your port:
```bash
ls /dev/ttyUSB* /dev/ttyACM*  # Linux
```

### Firebase

Update the database URL in `main.py`:
```python
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://your-project.firebaseio.com/'
})
```

## Running the Application

```bash
uv run python main.py
```

## Project Structure

```
api/
├── main.py              # Main application
├── agent-prompt.md      # Gemini system prompt
├── service_account.json # Firebase credentials
├── .env                 # Environment variables (API keys)
├── pyproject.toml       # Project dependencies
└── README.md
```

## Data Flow

### Input (from Arduino)
```json
{
  "co2": 300.0,
  "timestamp": "2026-01-24 17:38:02",
  "critical_traffic1": false,
  "critical_traffic2": false,
  "isLate": false
}
```

### Output (from Gemini)
```json
{
  "reasoning": "CO2 levels normal, no traffic congestion. Using default timings.",
  "timings": {
    "GREEN_TIME1": 5500,
    "GREEN_TIME2": 5500,
    "YELLOW_TIME1": 2000,
    "YELLOW_TIME2": 2000
  }
}
```

### Firebase Storage
```json
{
  "input": { /* Arduino data */ },
  "output": { /* Gemini response */ }
}
```

## Troubleshooting

### Serial Connection Failed
- Check Arduino is connected: `ls /dev/ttyUSB* /dev/ttyACM*`
- Grant permissions: `sudo chmod 666 /dev/ttyUSB0`
- Close Arduino IDE Serial Monitor (it locks the port)

### Gemini API Errors
- Verify API key in `.env`
- Check model name is valid (currently using `gemini-2.5-flash`)

### Firebase Errors
- Ensure Realtime Database is enabled (not Firestore)
- Verify `service_account.json` has correct project ID
- Check database rules allow write access

## Related Components

- **Frontend** (`../front/`): Web dashboard for data visualization
- **Arduino** (`../arduino/`): ESP32 firmware for sensor reading and traffic light control
