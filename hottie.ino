#include <TFT.h>  // Arduino LCD library
#include <SPI.h>
#include <stdlib.h>

// pin definition for the Uno
#define cs   10
#define dc   9
#define rst  8

TFT TFTscreen = TFT(cs, dc, rst);

char sensorPrintout[4];

const int sensorPin = A0;
const int wantedTempPin = A1;
const int securityPin = A5;
const int relay1Pin = 3;
const int relay2Pin = 2;
const float temperatureThreshold = 0.8;

float desiredTemperature = 22.00;

const int roomLabelLeftMargin = 50;
const int roomLabelTopMargin = 0;
const int wantedLabelLeftMargin = 30;
const int wantedLabelTopMargin = 70;
const int roomValueLeftMargin = 22;
const int roomValueTopMargin = 20;
const int wantedValueLeftMargin = 22;
const int wantedValueTopMargin = 90;

boolean coldTime = false;
boolean relayIsOn = false;
int securityIsOk = 0;
float relayAberration;

void setup() {
  Serial.begin(9600);

  pinMode(securityPin, INPUT);
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  checkRelayChangesInTemperature();

  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);

  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(2);
  TFTscreen.text("Room", roomLabelLeftMargin, roomLabelTopMargin);
  TFTscreen.text("Wanted", wantedLabelLeftMargin, wantedLabelTopMargin);

  TFTscreen.setTextSize(5);
}

void loop() {
  securityIsOk = analogRead(securityPin);
  Serial.print("Security sensor: ");
  Serial.println(securityIsOk);

  int wantedRead = analogRead(wantedTempPin);
  desiredTemperature = mapFloat(wantedRead, 0, 1023, 18, 32);
  Serial.print("Desired temperature: ");
  Serial.println(desiredTemperature);

  char roomTempToPrint[10] = "";
  float temperature = getAverageTemperature();
  Serial.print("Room temperature: ");
  Serial.println(temperature);

  dtostrf(temperature, 1, 1, roomTempToPrint);
  updateTFTRoomTemperature(roomTempToPrint);

  char wantedTempToPrint[10] = "";
  dtostrf(desiredTemperature, 1, 1, wantedTempToPrint);
  updateTFTWantedTemperature(wantedTempToPrint);

  if (securityIsOk < 1000) {
    turnOffRelay();
  } else if (!relayIsOn && temperature < desiredTemperature) {
    turnOnRelay();
  } else if (relayIsOn && temperature > desiredTemperature + temperatureThreshold) {
    turnOffRelay();
  }

  updateColdTimeValue();

  Serial.println("");
  delay(500);
}

float getAverageTemperature() {
  int loops = 200;
  float total = 0;
  for (int x = 0; x < loops; x++) {
    total = total + getTemperature();
    delay(1);
  }
  if (relayIsOn) {
    total -= relayAberration * loops;
  }
  return total / loops;
}

float getTemperature() {
  int sensorVal = analogRead(sensorPin);
  float voltage = (sensorVal / 1024.0) * 5.0;
  float temperature = (voltage - .5) * 100;
  if (relayIsOn) {
    temperature += 2;
  }
  return temperature;
}

char lastPrintedRoomText[10];
void updateTFTRoomTemperature(char text[10]) {
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.text(lastPrintedRoomText, roomValueLeftMargin, roomValueTopMargin);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(text, roomValueLeftMargin, roomValueTopMargin);
  strncpy(lastPrintedRoomText, text, 10);
}

char lastPrintedWantedText[10];
void updateTFTWantedTemperature(char text[10]) {
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.text(lastPrintedWantedText, wantedValueLeftMargin, wantedValueTopMargin);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(text, wantedValueLeftMargin, wantedValueTopMargin);
  strncpy(lastPrintedWantedText, text, 10);
}

void updateColdTimeValue() {
  //Serial.println(millis() % 60000);
  if (millis() % 60000 > 50000) {
    coldTime = true;
  } else {
    coldTime = false;
  }
}

void turnOnRelay() {
  digitalWrite(relay1Pin, HIGH);
  digitalWrite(relay2Pin, HIGH);
  relayIsOn = true;
}

void turnOffRelay() {
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);
  relayIsOn = false;
}

void checkRelayChangesInTemperature() {
  delay(200);
  float temperatureWithoutRelays = getAverageTemperature();
  turnOnRelay();
  delay(200);
  float temperatureWithRelays = getAverageTemperature();
  relayAberration = temperatureWithRelays - temperatureWithoutRelays;
}

float mapFloat(long x, long in_min, long in_max, long out_min, long out_max) {
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}


