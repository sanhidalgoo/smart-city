#include "Street.h"
#include "driver/gpio.h"

#include <Arduino.h>

Street::Street(int p1, int p2, int p3, unsigned long criticTime) {
    pin1 = p1; pin2 = p2; pin3 = p3;
    criticalTime = 7000;
    traffic = false;
  }

void Street::reset() {
  // "Reset" the JTAG pins so we can use them as normal GPIOs
  gpio_reset_pin((gpio_num_t)pin1);
  gpio_reset_pin((gpio_num_t)pin2);
  gpio_reset_pin((gpio_num_t)pin3);
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
}

bool Street::hasCriticalTraffic() {
  int in1 = digitalRead(pin1);
  int in2 = digitalRead(pin2);
  int in3 = digitalRead(pin3);
  if (!in1 || !in2 || !in3) {
    if (!traffic) {
      traffic = true;
      initTrafficTime = millis();
    } else {
      if ((millis() - initTrafficTime) > criticalTime) {
        return true;
      }
    }
  } else {
    traffic = false;
  }
  return false;
}
