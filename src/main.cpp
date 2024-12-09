#include <Arduino.h>
#include "DHT.h"
#include <TFT_eSPI.h>
#include <stdint.h>

#define GREEN_LED 33
#define MOTION_SENSOR 25

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

int motion_sensor_state = LOW;
int motion_sensor_val = 0;

void drawGauge(int x, int y, float value, const char *unit, uint16_t color, const char *label);
uint16_t getCurrentTempColor(int current, int desired);
uint16_t getDesiredTempColor(int desired);
void drawArc(int x, int y, int r, float startAngle, float endAngle, uint16_t color);

void setup() {
  Serial.begin(9600);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(MOTION_SENSOR, INPUT);

  dht.begin();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Display Initialized");
  delay(5000);
}

void loop() {

  static int prev_current_temp = -999;
  static int prev_desired_temp = -999;

  if (digitalRead(BLUE_BUTTON) == LOW){
    desired_temp -= 1;
    delay(500);
  }
  else if (digitalRead(RED_BUTTON) == LOW){
    desired_temp += 1;
    delay(500);
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

  if(current_temp != prev_current_temp || desired_temp != prev_current_temp)
  {
    uint16_t currentColor = getCurrentTempColor(current_temp, desired_temp);
    uint16_t desiredColor = getDesiredTempColor(desired_temp);
  
    tft.fillScreen(TFT_BLACK);

    drawGauge(67, 70, current_temp, "F", currentColor, "Current");
    drawGauge(180, 70, desired_temp, "F", desiredColor, "Desired");
  
    prev_current_temp = current_temp;
    prev_desired_temp = desired_temp;

    Serial.print("Current Temperature: "); Serial.print(current_temp); Serial.println(" degrees F");
    Serial.print("Desired Temperature: "); Serial.print(desired_temp); Serial.println(" degrees F");
  }

//motion sensor stuff
motion_sensor_val = digitalRead(MOTION_SENSOR);
if(motion_sensor_val == HIGH)
{
  digitalWrite(GREEN_LED, HIGH);
  //delay(500);

  if(motion_sensor_state == LOW)
  {
    Serial.println("Motion Detected!");
    motion_sensor_state = HIGH;
  }
}
else{
  digitalWrite(GREEN_LED, LOW);
  //delay(500);

  if(motion_sensor_state == HIGH)
  {
    Serial.println("Motion Not Detected!");
    motion_sensor_state = LOW;
  }
}

  

  //delay(500);
  /*
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.printf("Current Temp: %d F", current_temp);
  tft.setCursor(0, 50); // Move cursor to next line?
  tft.printf("Desired Temp: %d F", desired_temp);

  delay(500);
  */
}

void drawGauge(int x, int y, float value, const char *unit, uint16_t color, const char *label)
{
  int radius = 45;
  float angle = map(value, 32, 100, 135, 405);

  tft.fillCircle(x, y, radius, TFT_BLACK);
  tft.drawCircle(x, y, radius, TFT_WHITE);

  drawArc(x, y, radius - 10, 135, angle, color);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(String((int)value) + unit, x, y - 10, 2);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(label, x, y + 25, 2);
}

uint16_t getColor(int value)
{
  if(value <= 60)
  {
    return TFT_BLUE;
  }

  else if(value >= 80)
  {
    return TFT_RED;
  }
  return tft.color565((value - 60) *12, (80 - value) * 12, 255);
}

uint16_t getCurrentTempColor(int current, int desired)
{
  if(current < desired)
  {
    return TFT_RED;
  }
  else if ( current > desired)
  {
    return TFT_BLUE;
  }
  return TFT_GREEN;
}

uint16_t getDesiredTempColor(int desired)
{
  if(desired <= 72)
  {
    return TFT_BLUE;
  }
  else{
    return TFT_RED;
  }
}

void drawArc(int x, int y, int r, float startAngle, float endAngle, uint16_t color)
{
  for(int thickness = 0; thickness < 6; thickness++)
  {
    for(float angle = startAngle; angle < endAngle; angle += 1)
    {
      int x1 = x + (r + thickness) * cos(radians(angle));
      int y1 = y + (r + thickness) * sin(radians(angle));
      int x2 = x + (r + thickness) * cos(radians(angle + 1));
      int y2 = y + (r + thickness) * sin(radians(angle + 1));
      tft.drawLine(x1, y1, x2, y2, color);
    }
  }
}