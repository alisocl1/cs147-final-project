#include <Arduino.h>
#include "DHT.h"
#include <TFT_eSPI.h>

#define BLUE_LED 21
#define RED_LED 17
#define BLUE_BUTTON 2
#define RED_BUTTON 15
#define DHT_PIN 22
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);
TFT_eSPI tft = TFT_eSPI();

int current_temp;
int desired_temp = 72;

void setup() {
  Serial.begin(9600);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);

  dht.begin();

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
}

void loop() {
  if (digitalRead(BLUE_BUTTON) == HIGH){
    desired_temp -= 1;
    delay(10);
  }
  else if (digitalRead(RED_BUTTON) == HIGH){
    desired_temp += 1;
    delay(10);
  }

  if (current_temp > desired_temp){
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }
  else if (desired_temp > current_temp){
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  }
  else{
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(RED_LED, LOW);
  }
  current_temp = dht.readTemperature(true); //fahrenheit

  Serial.print("Temperature: "); Serial.print(current_temp); Serial.println(" degrees F");

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.printf("Current Temp: %d F", current_temp);
  tft.setCursor(0, 50); // Move cursor to next line?
  tft.printf("Desired Temp: %d F", desired_temp);

}
