#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "Button.h"

enum State {
  RED,
  GREEN,
  YELLOW
};

class Semaphore {
private:
  int redPin;
  int yellowPin;
  int greenPin;

public:
  Semaphore(int red, int yellow, int green, Button b1);
  Button pedestrianButton;
  void initialize();
  void setGreen();
  void setYellow();
  void setRed();
};

#endif