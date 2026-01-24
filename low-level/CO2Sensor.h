#ifndef CO2SENSOR_H
#define CO2SENSOR_H

#include <Arduino.h>

class CO2Sensor {
private:
  int pin;
  int currentLevel;
  int threshold;
  unsigned long lastUpdate;
  unsigned long updateInterval;
  
  // Simulación
  bool simulationMode;
  int simulatedDirection;
  int simulatedStep;

public:
  CO2Sensor(int pin, int threshold = 600);
  
  void update();
  bool isHigh();
  int getLevel();
  
  // Simulación
  void enableSimulation(int step = 10, unsigned long interval = 500);
  void setSimulatedLevel(int level);
};

#endif