# Smart City Simulation

A Cyber Physical Systems project implementing an intelligent traffic control system with CO2 monitoring and AI-powered timing optimization.

## Project Structure

This repository contains three implementation levels of the same traffic control system, each with increasing complexity and features:

```
├── low-level/          # Basic implementation - direct hardware control
├── mid-level/          # Intermediate - modular OOP design
└── high-level/         # Advanced - AI integration & cloud connectivity
    ├── arduino/        # ESP32 firmware with serial communication
    ├── api/            # Python backend with Gemini AI & Firebase
    └── front/          # Web dashboard for monitoring
```

### Low-Level

Basic traffic light controller with direct pin manipulation. Demonstrates fundamental embedded programming concepts without abstraction layers.

### Mid-Level

Object-oriented approach with modular components:
- `TrafficSemaphore` - Traffic light state management
- `Street` - Street coordination logic
- `CO2Sensor` - Air quality monitoring
- `LightSensors` - Traffic density detection
- `Button` - Manual override controls

### High-Level

Full-stack smart city solution featuring:
- **Arduino (ESP32)**: Sensor data collection and traffic light control with serial communication
- **Python API**: Bridge between hardware and AI, handles Gemini AI requests and Firebase storage
- **Frontend**: Real-time dashboard for monitoring traffic and environmental data

## Hardware Components

- ESP32 microcontroller
- Traffic light LEDs (Red, Yellow, Green)
- MQ-135 CO2 sensor
- LDR light sensors for traffic detection
- Push buttons for manual control

## Architecture (High-Level)

```
Arduino (ESP32) <--Serial--> Python API <--HTTP--> Gemini AI
                                  |
                                  v
                          Firebase RTDB
                                  ^
                                  |
                            Web Dashboard
```

## Getting Started

Each level can be run independently. See the README in each directory for specific setup instructions.

For the high-level implementation:
1. Flash `high-level/arduino/` to your ESP32
2. Configure and run `high-level/api/` Python server
3. Open `high-level/front/` dashboard in your browser

## License

Educational project for Cyber Physical Systems coursework.
