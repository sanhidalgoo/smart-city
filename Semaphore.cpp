#include <Arduino.h>
#include "Semaphore.h"

Semaphore::Semaphore(int red, int yellow, int green, Button b1) : pedestrianButton(b1) {
  redPin = red;
  yellowPin = yellow;
  greenPin = green;

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

  initialize();
}

void Semaphore::initialize() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
}

void Semaphore::setGreen() {
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, HIGH);
}

void Semaphore::setYellow() {
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
}

void Semaphore::setRed() {
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
}