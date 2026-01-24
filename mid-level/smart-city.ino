#include <Wire.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include "Button.h"
#include "TrafficSemaphore.h"
#include "Street.h"
#include "CO2Sensor.h"
#include "LightSensors.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// WIFI setup
#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define DATABASE_URL "" 
#define USER_EMAIL ""
#define USER_PASSWORD ""

// NTP
#include <time.h>

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
#define BASE_PEDESTRIAN_DEBOUNCE_DELAY 500
#define BASE_DEBOUNCE_DELAY 500
#define LCD_UPDATE_INTERVAL 500
#define YELLOW_BLINK_TIME 1000
#define BASE_C02_THRESHOLD 400

bool extraGreen1 = false;
bool extraGreen2 = false;
int co2Threshold = BASE_C02_THRESHOLD;
int pedestrianBounceDelay = BASE_PEDESTRIAN_DEBOUNCE_DELAY;

// ------------------------------------------------------------- //

Button button1(P2, BASE_DEBOUNCE_DELAY);
Button button2(P1, BASE_DEBOUNCE_DELAY);
TrafficSemaphore light1(LR1, LY1, LG1, button1);  // Semáforo 1
TrafficSemaphore light2(LR2, LY2, LG2, button2);  // Semáforo 2
Street street1(CNY1, CNY2, CNY3, 7000);           // Street 1
Street street2(CNY4, CNY5, CNY6, 7000);           // Street 2
CO2Sensor co2Sensor(CO2_PIN);
LightSensors lightSensors(LDR1, LDR2, 2000);

LiquidCrystal_I2C lcd(0x27, 16, 4);

// NTP configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;  // UTC-5 (Colombia)
const int daylightOffset_sec = 0;
unsigned long sendDataPrevMillis = 0;

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

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void updateLCD() {
  bool highCO2 = co2Sensor.isHigh(co2Threshold);

  if (highCO2 == lastCO2State) return;

  lastCO2State = highCO2;
  lcd.clear();

  if (highCO2) {
    lcd.setCursor(0, 0);
    lcd.print(getTimestamp());
    lcd.setCursor(0, 1);
    lcd.print("!! ALERTA CO2 !!");
    lcd.setCursor(0, 2);
    lcd.print("Tome rutas");
    lcd.setCursor(0, 3);
    lcd.print("alternas");
  } else {
    lcd.setCursor(0, 0);
    lcd.print(getTimestamp());
    lcd.setCursor(0, 1);
    lcd.print("CO2: concentracion");
    lcd.setCursor(0, 2);
    lcd.print("normal");
  }
}

void sendData() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 10000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    float co2Value = co2Sensor.getLevel();
    String timestamp = getTimestamp();

    Serial.printf("CO2: %.2f ppm - Fecha: %s\n", co2Value, timestamp.c_str());

    FirebaseJson json;
    json.set("co2", co2Value);
    json.set("timestamp", timestamp);
    json.set("unix_time", (int)time(nullptr));  // timestamp Unix opcional
    
    Serial.printf("History: %s\n", 
      Firebase.RTDB.pushJSON(&fbdo, F("/co2/history"), &json) ? "ok" : fbdo.errorReason().c_str());
  }  
}

void setNextState(SemaphoreState nuevoEstado, unsigned long duracion) {
  currentState = nuevoEstado;
  stateDuration = duracion;
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "error";
  }
  
  char buffer[25];
  // Formato: 2025-01-22 14:30:45
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
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

  // WIFI connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; 
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  fbdo.setResponseSize(2048);
  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);
  config.timeout.serverResponse = 10 * 1000;

  co2Sensor.enableSimulation(15, 800);
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("CO2: concentracion");
  lcd.setCursor(0, 1);
  lcd.print("normal");
  updateLCD();

  lightSensors.start();

  // NTP sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Syncing with NTP...");
  
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("NTP in sync!");
}

void loop() {
  unsigned long currentMillis = millis();

  sendData();
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

  pedestrianBounceDelay = isLate ? BASE_PEDESTRIAN_DEBOUNCE_DELAY : BASE_PEDESTRIAN_DEBOUNCE_DELAY*8; //BASE_DEBOUNCE_DELAY * 2;

  if (pedestrianDebounce && (currentMillis - previousMillis >= pedestrianBounceDelay)) pedestrianDebounce = false;

  if (!pedestrianDebounce && (button1.wasPressed() || button2.wasPressed())) {
    if (currentState == GREEN1_RED2 || currentState == RED1_GREEN2)
      stateDuration = 0;

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

  if (isLate){
    co2Threshold = 700;
    if(co2Sensor.isHigh(co2Threshold) && (currentState == GREEN1_RED2 || currentState == YELLOW1_RED2)) {
      Serial.println("Extending green time for semaphore 2 due to high CO2");
      extraGreen2 = true; // Add more time to semaphore 2 green light (where tunnel is)
      if (currentState == GREEN1_RED2) stateDuration = 0; // Force Light 1 to Yellow immediately
    }
  } else if (co2Threshold != BASE_C02_THRESHOLD) {
    co2Threshold = BASE_C02_THRESHOLD;
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

        if (!isLate) {
          setNextState(GREEN1_RED2, 0);
          isBlinkOn = false;
        }
        break;
    }
  }
}
