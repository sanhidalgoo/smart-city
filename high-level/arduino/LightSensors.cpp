#include "LightSensors.h"
#include <Arduino.h>

LightSensors::LightSensors(int p1, int p2, unsigned long criticTime) {
  pin1 = p1; pin2 = p2, criticalTime = criticTime;
}

void LightSensors::start() {
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
}

bool LightSensors::isDark() {
  int sensor1 = analogRead(pin1);
  int sensor2 = analogRead(pin2);
  // Serial.print("Light 1: ");
  // Serial.println(sensor1);
  // Serial.print("Light 2: ");
  // Serial.println(sensor2);
  if (sensor1 < 300 && sensor2 < 300) {
    return true;
  }
  return false;
}
