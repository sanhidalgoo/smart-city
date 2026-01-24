#include "CO2Sensor.h"

CO2Sensor::CO2Sensor(int pin, int threshold) {
  this->pin = pin;
  this->threshold = threshold;
  this->currentLevel = 300;
  this->lastUpdate = 0;
  this->updateInterval = 500;
  this->simulationMode = false;
  this->simulatedDirection = 1;
  this->simulatedStep = 10;
  
  pinMode(pin, INPUT);
}

void CO2Sensor::update() {
  unsigned long now = millis();
  
  if (simulationMode && (now - lastUpdate >= updateInterval)) {
    lastUpdate = now;
    
    currentLevel += simulatedDirection * simulatedStep;
    
    if (currentLevel >= 900) {
      currentLevel = 900;
      simulatedDirection = -1;
    } else if (currentLevel <= 200) {
      currentLevel = 200;
      simulatedDirection = 1;
    }
  } else if (!simulationMode) {
    currentLevel = analogRead(pin);
  }
}

bool CO2Sensor::isHigh() {
  return currentLevel > threshold;
}

int CO2Sensor::getLevel() {
  return currentLevel;
}

void CO2Sensor::enableSimulation(int step, unsigned long interval) {
  simulationMode = true;
  simulatedStep = step;
  updateInterval = interval;
}

void CO2Sensor::setSimulatedLevel(int level) {
  currentLevel = constrain(level, 0, 1023);
}