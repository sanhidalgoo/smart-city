#ifndef TRAFFICSEMAPHORE_H
#define TRAFFICSEMAPHORE_H

#include <Arduino.h>
#include "Button.h"

enum State {
  RED,
  GREEN,
  YELLOW
};

class TrafficSemaphore {
private:
  int redPin;
  int yellowPin;
  int greenPin;
  Button pedestrianButton;

public:
  TrafficSemaphore(int red, int yellow, int green, Button& b1);
  void initialize();
  void setGreen();
  void setYellow();
  void setRed();
};

#endif
