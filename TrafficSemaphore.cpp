#include "TrafficSemaphore.h"

TrafficSemaphore::TrafficSemaphore(
  int red,
  int yellow,
  int green,
  Button& b1)
  : pedestrianButton(b1) {
  redPin = red;
  yellowPin = yellow;
  greenPin = green;

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  initialize();
}

void TrafficSemaphore::initialize() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
}

void TrafficSemaphore::setGreen() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, HIGH);
}

void TrafficSemaphore::setYellow() {
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
}

void TrafficSemaphore::setRed() {
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
}
