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
const int relayPin = 3;

const int desiredTemperature = 25;

const int roomLabelLeftMargin = 50;
const int roomLabelTopMargin = 0;
const int wantedLabelLeftMargin = 30;
const int wantedLabelTopMargin = 60;
const int roomValueLeftMargin = 6;
const int roomValueTopMargin = 20;
const int wantedValueLeftMargin = 6;
const int wantedValueTopMargin = 80;

boolean coldTime = false;
boolean relayIsOn = false;

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);

  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);

  TFTscreen.stroke(255, 255, 255);
  TFTscreen.setTextSize(2);
  TFTscreen.text("Room", roomLabelLeftMargin, roomLabelTopMargin);
  TFTscreen.text("Wanted", wantedLabelLeftMargin, wantedLabelTopMargin);

  TFTscreen.setTextSize(5);
  TFTscreen.text("25.00", wantedValueLeftMargin, wantedValueTopMargin);
}

void loop() {
  char textToPrint[10] = "";
  float temperature = getAverageTemperature();
  dtostrf(temperature, 1, 2, textToPrint);

  updateTFTTemperature(textToPrint);

  if (temperature < desiredTemperature && !coldTime) {
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

char lastPrintedText[10];
void updateTFTTemperature(char text[10]) {
  TFTscreen.stroke(0, 0, 0);
  TFTscreen.text(lastPrintedText, roomValueLeftMargin, roomValueTopMargin);
  TFTscreen.stroke(255, 255, 255);
  TFTscreen.text(text, roomValueLeftMargin, roomValueTopMargin);
  strncpy(lastPrintedText, text, 10);
}

void updateColdTimeValue() {
  Serial.println(millis() % 60000);
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

