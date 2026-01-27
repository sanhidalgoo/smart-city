# Smart Traffic Controller Dashboard

A web-based dashboard for visualizing real-time traffic controller data and AI-powered timing decisions.

## Overview

This frontend displays data collected from the smart traffic intersection system, including:
- Sensor readings (CO2 levels, traffic status)
- AI reasoning for timing decisions
- Semaphore timing configurations

## Prerequisites

- Python 3.13+
- [uv](https://docs.astral.sh/uv/) package manager
- Firebase Realtime Database credentials (`service_account.json`)

## Installation

1. Navigate to the front directory:
   ```bash
   cd high-level/front
   ```

2. Install dependencies:
   ```bash
   uv sync
   ```

3. Ensure `service_account.json` is present in the directory (copy from `../api/` if needed):
   ```bash
   cp ../api/service_account.json .
   ```

## Running the Application

Start the Flask development server:
```bash
uv run python main.py
```

Open your browser at: http://localhost:5000

## Project Structure

```
front/
├── main.py              # Flask application
├── service_account.json # Firebase credentials
├── templates/
│   └── index.html       # Dashboard template
├── pyproject.toml       # Project dependencies
└── README.md
```

## Data Displayed

| Column | Description |
|--------|-------------|
| Timestamp | Date/time of the reading |
| CO2 (ppm) | Air quality level (green < 400, yellow 400-700, red > 700) |
| Traffic 1/2 | Congestion status per street |
| Night Mode | Day/night detection from light sensors |
| Green T1/T2 | Green light duration per semaphore |
| Yellow T1/T2 | Yellow light duration per semaphore |
| AI Reasoning | Gemini's explanation for timing decisions |

## Configuration

To change the Firebase database URL, edit `main.py`:
```python
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://your-project.firebaseio.com/'
})
```

To change the server port:
```python
app.run(debug=True, port=5000)  # Change port here
```

## Related Components

- **API** (`../api/`): Python server that processes Arduino data and communicates with Gemini AI
- **Arduino** (`../arduino/`): ESP32 firmware for sensor reading and traffic light control
