#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Button.h"
#include "TrafficSemaphore.h"
#include "Street.h"
#include "CO2Sensor.h"
#include "LightSensors.h"

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


// -------------------------------------------------------------- //
//  STATE TIME VARIABLES

#define GREEN_TIME1 5500       // Green time for light 1
#define YELLOW_TIME1 2000       // Yellow time for light 1
#define RED_TIME1 6000         // Red time for light 1
#define GREEN_TIME2 5500       // Green time for light 2
#define YELLOW_TIME2 2000       // Yellow time for light 2
#define RED_TIME2 6000         // Red time for light 2
#define PEDESTRIAN_DEBOUNCE 500
#define DEBOUNCE_DELAY 500
#define CO2_THRESHOLD 600
#define LCD_UPDATE_INTERVAL 500
#define YELLOW_BLINK_TIME 1000

// Add these flags at the top with your other variables
bool extraGreen1 = false;
bool extraGreen2 = false;

// ------------------------------------------------------------- //

Button button1(P2, DEBOUNCE_DELAY);
Button button2(P1, DEBOUNCE_DELAY);
TrafficSemaphore light1(LR1, LY1, LG1, button1);  // Semáforo 1
TrafficSemaphore light2(LR2, LY2, LG2, button2);  // Semáforo 2
Street street1(CNY1, CNY2, CNY3, 7000);           // Street 1
Street street2(CNY4, CNY5, CNY6, 7000);           // Street 2
CO2Sensor co2Sensor(CO2_PIN, CO2_THRESHOLD);
LightSensors lightSensors(LDR1, LDR2, 2000);

LiquidCrystal_I2C lcd(0x27, 16, 4);

enum SemaphoreState {
  GREEN1_RED2,
  YELLOW1_RED2,
  RED1_GREEN2,
  RED1_YELLOW2,
  YELLOW1_YELLOW2
};

SemaphoreState currentState;
unsigned long previousMillis = 0;
unsigned long stateDuration = 0;
unsigned long lastLCDUpdate = 0;
bool pedestrianDebounce = false;
bool lastCO2State = false;
bool isBlinkOn = false;

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

void setNextState(SemaphoreState nuevoEstado, unsigned long duracion) {
  currentState = nuevoEstado;
  stateDuration = duracion;
}

void setup() {
  currentState = GREEN1_RED2;
  previousMillis = millis();
  stateDuration = GREEN_TIME1;

  light1.initialize();
  light1.setGreen();
  light2.initialize();
  light2.setRed();
  street1.reset();
  street2.reset();
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

  lightSensors.start();
}

void loop() {
  unsigned long currentMillis = millis();

  button1.update();
  button2.update();
  co2Sensor.update();

  if (currentMillis - lastLCDUpdate >= LCD_UPDATE_INTERVAL) {
    lastLCDUpdate = currentMillis;
    updateLCD();
  }

  // Identify critical traffic in each street
  bool traffic1 = street1.hasCriticalTraffic();
  bool traffic2 = street2.hasCriticalTraffic();
  bool isLate = lightSensors.isDark();

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

  if (isLate && currentState != YELLOW1_YELLOW2 && !traffic1 && !traffic2) {
    setNextState(YELLOW1_YELLOW2, 0);
    Serial.println("low traffic and in the night moving to yellow");
  }

  if (currentMillis - previousMillis >= stateDuration) {
    previousMillis = currentMillis;

    switch (currentState) {
      case GREEN1_RED2:
        light1.setYellow();
        light2.setRed();
        
        setNextState(YELLOW1_RED2, YELLOW_TIME1);
        break;

      case YELLOW1_RED2: {
        light1.setRed();
        light2.setGreen();

        unsigned long tiempoVerde = extraGreen2 ? (GREEN_TIME2 * 4) : GREEN_TIME2;
        extraGreen2 = false;

        setNextState(RED1_GREEN2, tiempoVerde);
        break;
      }
      case RED1_GREEN2:
        light1.setRed();
        light2.setYellow();

        setNextState(RED1_YELLOW2, YELLOW_TIME2);
        break;

      case RED1_YELLOW2: {
        light1.setGreen();
        light2.setRed();

        unsigned long tiempoVerde1 = extraGreen1 ? (GREEN_TIME1 * 4) : GREEN_TIME1;
        extraGreen1 = false; 

        setNextState(GREEN1_RED2, tiempoVerde1);
        break;
      }
      case YELLOW1_YELLOW2:
        if (isBlinkOn) {
          light1.setBlack();
          light2.setBlack();
        } else {
          light1.setYellow();
          light2.setYellow();
        }

        isBlinkOn = !isBlinkOn;
        stateDuration = YELLOW_BLINK_TIME;

        if((isLate && traffic1) || !isLate) {
          setNextState(RED1_GREEN2, 0);
          isBlinkOn = false;
          Serial.println("traffic in line 1 move green1_red2");
        } else if (isLate && traffic2) {
          setNextState(GREEN1_RED2, 0);
          isBlinkOn = false;
          Serial.println("traffic in line 2 move green2_red1");
        } 
        break;
    }
  }
}
