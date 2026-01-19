#include "Button.h"
#include "TrafficSemaphore.h"
#include "Street.h"

// I/O pin labeling
#define LDR1 12 // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 13 // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 14  // CO2 sensor connected in pin A3
#define P1 1    // Traffic light 1 button connected in pin 1
#define P2 2    // Traffic light 2 button connected in pin 2
#define CNY1 42 // Infrared sensor 1 in traffic light 1 connected in pin 42
#define CNY2 41 // Infrared sensor 2 in traffic light 1 connected in pin 41
#define CNY3 40 // Infrared sensor 3 in traffic light 1 connected in pin 40
#define CNY4 39 // Infrared sensor 4 in traffic light 2 connected in pin 39
#define CNY5 38 // Infrared sensor 5 in traffic light 2 connected in pin 38
#define CNY6 37 // Infrared sensor 6 in traffic light 2 connected in pin 37
#define LR1 5   // Red traffic light 1 connected in pin 5
#define LY1 4   // Yellow traffic light 1 connected in pin 4
#define LG1 6   // Green traffic light 1 connected in pin 6
#define LR2 7   // Red traffic light 2 connected in pin 7
#define LY2 15  // Yellow traffic light 2 connected in pin 15
#define LG2 16  // Green traffic light 2 connected in pin 16


// -------------------------------------------------------------- //
//  STATE TIME VARIABLES

#define GREEN_TIME1 5500       // Green time for light 1
#define YELLOW_TIME1 2000       // Yellow time for light 1
#define RED_TIME1 6000         // Red time for light 1
#define GREEN_TIME2 5500       // Green time for light 2
#define YELLOW_TIME2 2000       // Yellow time for light 2
#define RED_TIME2 6000         // Red time for light 2
#define PEDESTRIAN_DEBOUNCE 500

#define DEBOUNCE_DELAY 500  // Debounce delay in milliseconds

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

enum SemaphoreState {
  GREEN1_RED2,
  YELLOW1_RED2,
  RED1_GREEN2,
  RED1_YELLOW2
};

SemaphoreState currentState;
unsigned long previousMillis = 0;
unsigned long stateDuration = 0;
bool pedestrianDebounce = false;

void setup() {
  // Inicializa la máquina de estados
  currentState = GREEN1_RED2;
  previousMillis = millis();
  stateDuration = GREEN_TIME1;  // Initial green time for light 1

  light1.initialize();
  light1.setGreen();
  light2.initialize();
  light2.setRed();
  street1.reset();
  street2.reset();
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // Update button states
  button1.update();
  button2.update();

  // Identify critical traffic in each street
  bool traffic1 = street1.hasCriticalTraffic();
  bool traffic2 = street2.hasCriticalTraffic();
  
  if (pedestrianDebounce && (currentMillis - previousMillis >= PEDESTRIAN_DEBOUNCE)) pedestrianDebounce = false;
  // Handle button presses to transition faster
  if (!pedestrianDebounce && (button1.wasPressed() || button2.wasPressed())) {

    if (currentState == GREEN1_RED2 || currentState == RED1_GREEN2)
      stateDuration = 0;  // Transition to yellow immediately

    Serial.println(currentState);
    Serial.println(stateDuration);
    button1.reset();
    button2.reset();
    pedestrianDebounce = true;
  } else if (traffic1 && (currentState == RED1_GREEN2 || currentState == RED1_YELLOW2)) {
      extraGreen1 = true; 
      if (currentState == RED1_GREEN2) stateDuration = 0; // Force Light 2 to Yellow immediately
  } else if (traffic2 && (currentState == GREEN1_RED2 || currentState == YELLOW1_RED2)) {
      extraGreen2 = true;
      if (currentState == GREEN1_RED2) stateDuration = 0; // Force Light 1 to Yellow immediately
  }

  // Update the state machine
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

        if (extraGreen2) {
            stateDuration = GREEN_TIME2 * 4; // 4x duration to clear traffic
            extraGreen2 = false;            // Reset flag
            Serial.println("EXTENDED GREEN ACTIVE FOR STREET 2");
        } else {
            stateDuration = GREEN_TIME2;
        }
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
        
        // CHECK FOR MULTIPLIER
        if (extraGreen1) {
            stateDuration = GREEN_TIME1 * 4; // 4x duration to clear traffic
            extraGreen1 = false;            // Reset flag
            Serial.println("EXTENDED GREEN ACTIVE FOR STREET 1");
        } else {
            stateDuration = GREEN_TIME1;
        }
        break;
    }
  }
}
