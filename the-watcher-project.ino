/*************************************************************

  You’ll need:
   - Blynk IoT app (download from App Store or Google Play)
   - NodeMCU board
   - Decide how to connect to Blynk
     (USB, Ethernet, Wi-Fi, Bluetooth, ...)

  There is a bunch of great example sketches included to show you how to get
  started. Think of them as LEGO bricks  and combine them as you wish.
  For example, take the Ethernet Shield sketch and combine it with the
  Servo example, or choose a USB sketch and add a code from SendData
  example.
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
#define DHTPIN 3 //
#define DHTTYPE DHT11


// Your WiFi credentials for Blynk App.
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "";// Wifi Name
char pass[] = ""; //Wifi Password
BlynkTimer timer;

DHT dht(DHTPIN, DHTTYPE);

//PIN setting for Relay Switch
const int PIN_TO_RELAY = 0; //

//PIN setting for Motion Sesion
const int PIN_TO_SENSOR = 4;   //

//PIN Settings for RGB Module
const int PIN_TO_RED = 1; //
const int PIN_TO_GREEN = 5; // 
const int PIN_TO_BLUE = 16; // 

//PIN setting for Ultrasonic Sensor
const int PIN_TO_TRIGGER = 12;
const int PIN_TO_ECHO = 14;

void setup()
{
  //Serialization begin
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  //Pinmodes
  pinMode(PIN_TO_RELAY, OUTPUT);
  pinMode(PIN_TO_RED, OUTPUT);
  pinMode(PIN_TO_GREEN, OUTPUT);
  pinMode(PIN_TO_BLUE, OUTPUT);
  pinMode(PIN_TO_TRIGGER, OUTPUT);
  
  pinMode(PIN_TO_SENSOR, INPUT);
  pinMode(PIN_TO_ECHO, INPUT);
}


//DHT11 Sensor Setting
void monitorTempAndHumid(){
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Blynk.virtualWrite(V5, h);
  Blynk.virtualWrite(V4, t);
}


//Enable Watcher Settings
bool enabledWatcher = false;
bool delayEnabled = false;
const unsigned long DELAY_TIME_MS = 60000; // 30000 miliseconds ~ 30 seconds
unsigned long delayStartTime;

BLYNK_WRITE(V3){
  int pinValue = param.asInt();
  if(pinValue == 1){
    enabledWatcher = true;
  }else{
    enabledWatcher = false;
  }
}

//Ultrasonic Sensor Settings
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
bool objectStatePrevious = false;
bool objectStateCurrent = false;

void detectObject() {
  if(enabledWatcher){
    digitalWrite(PIN_TO_TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_TO_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TO_TRIGGER, LOW);
    duration = pulseIn(PIN_TO_ECHO, HIGH);
    distanceCm = duration * SOUND_VELOCITY/2;
    distanceInch = distanceCm * CM_TO_INCH;
    
    objectStatePrevious = objectStateCurrent;
    objectStateCurrent = distanceInch < 36;

    if(objectStatePrevious == false && objectStateCurrent == true){
      delayEnabled = false;
      digitalWrite(PIN_TO_RELAY, HIGH);
      Blynk.virtualWrite(V7, 1);
      Blynk.virtualWrite(V11, distanceInch);
      Blynk.virtualWrite(V12, "Alert! Object detected!");
    }else if(objectStatePrevious == true && objectStateCurrent == false){
      delayEnabled = true;
      delayStartTime = millis();
    }

    if (delayEnabled == true && (millis() - delayStartTime) >= DELAY_TIME_MS) {
      delayEnabled = false; // disable delay
      digitalWrite(PIN_TO_RELAY, LOW);
      Blynk.virtualWrite(V7, 0);
      Blynk.virtualWrite(V11, distanceInch);
      Blynk.virtualWrite(V12, "");
    }
  }
  
}

//Motion Sensor Setting
int pinStateCurrent   = LOW;
int pinStatePrevious  = LOW;

void detectMotion(){
  
  if(enabledWatcher){
    pinStatePrevious = pinStateCurrent;
    pinStateCurrent = digitalRead(PIN_TO_SENSOR);
  
    if (pinStatePrevious == LOW && pinStateCurrent == HIGH) {
      delayEnabled = false;
      digitalWrite(PIN_TO_RELAY, HIGH);
      Blynk.virtualWrite(V7, 1);
      Blynk.virtualWrite(V13, "Alert! Motion detected!");
      
    }else if (pinStatePrevious == HIGH && pinStateCurrent == LOW) {
      delayEnabled = true;
      delayStartTime = millis();
    }
  
    if (delayEnabled == true && (millis() - delayStartTime) >= DELAY_TIME_MS) {
      delayEnabled = false;
      digitalWrite(PIN_TO_RELAY, LOW);
      Blynk.virtualWrite(V7, 0);
      Blynk.virtualWrite(V13, "");
    }
  }
}


BLYNK_WRITE(V7){
  int pinValue = param.asInt();
  if(pinValue == 0){
    digitalWrite(PIN_TO_RELAY, LOW);
    Blynk.virtualWrite(V12, "");
    Blynk.virtualWrite(V13, "");
  }else{
    digitalWrite(PIN_TO_RELAY, HIGH);
  }
}

//RGB LED Module Setting
int P_RED = 0;
int P_GREEN = 0;
int P_BLUE = 0;

void controlRGBColor() {
  analogWrite(PIN_TO_RED,   P_RED);
  analogWrite(PIN_TO_GREEN, P_GREEN);
  analogWrite(PIN_TO_BLUE,  P_BLUE);
}

BLYNK_WRITE(V8){
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable 
  P_RED = pinValue;
}

BLYNK_WRITE(V9){
  P_GREEN = param.asInt(); // assigning incoming value from pin V1 to a variable 
}

BLYNK_WRITE(V10){
  P_BLUE = param.asInt(); // assigning incoming value from pin V1 to a variable 
}

//Loop Function

void loop()
{
  Blynk.run();
  monitorTempAndHumid();
  detectObject();
  detectMotion();
  controlRGBColor();
}
