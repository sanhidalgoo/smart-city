#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Button.h"
#include "TrafficSemaphore.h"
#include "CO2Sensor.h"

// I/O pin labeling
#define LDR1 12
#define LDR2 13
#define CO2_PIN 14
#define CO2 14
#define P1 1
#define P2 2
#define CNY1 42
#define CNY2 41
#define CNY3 40
#define CNY4 39
#define CNY5 38
#define CNY6 37
#define LR1 5
#define LY1 4
#define LG1 6
#define LR2 7
#define LY2 15
#define LG2 16

#define GREEN_TIME1 5500
#define YELLOW_TIME1 2000
#define RED_TIME1 6000
#define GREEN_TIME2 5500
#define YELLOW_TIME2 2000
#define RED_TIME2 6000
#define PEDESTRIAN_DEBOUNCE 500
#define DEBOUNCE_DELAY 500
#define CO2_THRESHOLD 600
#define LCD_UPDATE_INTERVAL 500

Button button1(P2, DEBOUNCE_DELAY);
Button button2(P1, DEBOUNCE_DELAY);
TrafficSemaphore light1(LR1, LY1, LG1, button1);
TrafficSemaphore light2(LR2, LY2, LG2, button2);
CO2Sensor co2Sensor(CO2_PIN, CO2_THRESHOLD);

LiquidCrystal_I2C lcd(0x27, 16, 4);

enum SemaphoreState {
  GREEN1_RED2,
  YELLOW1_RED2,
  RED1_GREEN2,
  RED1_YELLOW2
};

SemaphoreState currentState;
unsigned long previousMillis = 0;
unsigned long stateDuration = 0;
unsigned long lastLCDUpdate = 0;
bool pedestrianDebounce = false;
bool lastCO2State = false;

void updateLCD() {
  bool highCO2 = co2Sensor.isHigh();
  
  // Solo actualizar si cambió el estado
  if (highCO2 == lastCO2State) return;
  
  lastCO2State = highCO2;
  lcd.clear();
  
  if (highCO2) {
    lcd.setCursor(0, 0);
    lcd.print("!! ALERTA CO2 !!");
    lcd.setCursor(0, 1);
    lcd.print("Tome rutas");
    lcd.setCursor(0, 2);
    lcd.print("alternas");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("CO2: concentracion");
    lcd.setCursor(0, 1);
    lcd.print("normal");
  }
}

void setup() {
  currentState = GREEN1_RED2;
  previousMillis = millis();
  stateDuration = GREEN_TIME1;

  light1.initialize();
  light1.setGreen();
  light2.initialize();
  light2.setRed();
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  co2Sensor.enableSimulation(15, 800);
  
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("CO2: concentracion");
  lcd.setCursor(0, 1);
  lcd.print("normal");
  updateLCD();
}

void loop() {
  unsigned long currentMillis = millis();

  button1.update();
  button2.update();
  co2Sensor.update();

  if (currentMillis - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = currentMillis;
    updateLCD();
    
    Serial.print("CO2: ");
    Serial.print(co2Sensor.getLevel());
    Serial.println(co2Sensor.isHigh() ? " - ALTO!" : " - Normal");
  }

  if (pedestrianDebounce && (currentMillis - previousMillis >= PEDESTRIAN_DEBOUNCE)) pedestrianDebounce = false;

  if (!pedestrianDebounce && (button1.wasPressed() || button2.wasPressed())) {
    if (currentState == GREEN1_RED2 || currentState == RED1_GREEN2)
      stateDuration = 0;

    Serial.println(currentState);
    Serial.println(stateDuration);
    button1.reset();
    button2.reset();
    pedestrianDebounce = true;
  }

  if (currentMillis - previousMillis >= stateDuration) {
    previousMillis = currentMillis;

    switch (currentState) {
      case GREEN1_RED2:
        light1.setYellow();
        light2.setRed();
        currentState = YELLOW1_RED2;
        stateDuration = YELLOW_TIME1;
        break;

      case YELLOW1_RED2:
        light1.setRed();
        light2.setGreen();
        currentState = RED1_GREEN2;
        stateDuration = GREEN_TIME2;
        break;

      case RED1_GREEN2:
        light1.setRed();
        light2.setYellow();
        currentState = RED1_YELLOW2;
        stateDuration = YELLOW_TIME2;
        break;

      case RED1_YELLOW2:
        light1.setGreen();
        light2.setRed();
        currentState = GREEN1_RED2;
        stateDuration = GREEN_TIME1;
        break;
    }
  }
}