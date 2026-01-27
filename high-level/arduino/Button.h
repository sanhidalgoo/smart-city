#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
private:
  int pin;
  bool wasPressedInCycle;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;

public:
  Button(int pin, unsigned long debounceDelay = 50);
  void update();
  bool wasPressed();
  void reset();
  void updateDebounceDelay(unsigned long debounceDelay);
};

#endif
