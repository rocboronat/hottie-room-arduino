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
const int relayPin = 3;

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

void setup() {
  Serial.begin(9600);

  pinMode(securityPin, INPUT);
  pinMode(relayPin, OUTPUT);

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
  int wantedRead = analogRead(wantedTempPin);
  desiredTemperature = mapFloat(wantedRead, 0, 1023, 18, 32);
  Serial.println(desiredTemperature);

  char roomTempToPrint[10] = "";
  float temperature = getAverageTemperature();
  dtostrf(temperature, 1, 1, roomTempToPrint);
  updateTFTTemperature(roomTempToPrint);

  char wantedTempToPrint[10] = "";
  dtostrf(desiredTemperature, 1, 1, wantedTempToPrint);
  updateTFTWantedTemperature(wantedTempToPrint);

  if (securityIsOk == 1023 && temperature < desiredTemperature && !coldTime) {
    turnOnRelay();
  } else {
    turnOffRelay();
  }

  updateColdTimeValue();

  delay(500);
}

float getAverageTemperature() {
  int loops = 100;
  float total = 0;
  for (int x = 0; x < loops; x++) {
    total = total + getTemperature();
  }
  if (relayIsOn) {
    total -= 3.20 * loops;
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
void updateTFTTemperature(char text[10]) {
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
  digitalWrite(relayPin, HIGH);
  relayIsOn = true;
}

void turnOffRelay() {
  digitalWrite(relayPin, LOW);
  relayIsOn = false;
}


float mapFloat(long x, long in_min, long in_max, long out_min, long out_max) {
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}


