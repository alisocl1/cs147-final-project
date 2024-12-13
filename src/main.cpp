#include <Arduino.h>
#include "DHT.h"
#include <TFT_eSPI.h>
#include <stdint.h>
#include <WiFi.h>
#include <HttpClient.h>

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
unsigned long lastReadTime = 0;
const unsigned long readInterval = 5000;

static int prev_current_temp = -999;
static int prev_desired_temp = -999;

static const char* ssid = "alison phone";
static const char* password = "-";
const int kNetworkTimeout = 30 * 1000;
const int kNetworkDelay = 1000;

void drawGauge(int x, int y, float value, const char *unit, uint16_t color, const char *label);
uint16_t getCurrentTempColor(int current, int desired);
uint16_t getDesiredTempColor(int desired);
void drawArc(int x, int y, int r, float startAngle, float endAngle, uint16_t color);

// Handle GET request to AWS server
void toAWS(float t, float h){
  String humidity = String(h, 2);
  String temperature = String(t, 2);

  String url = "/?humidity=" + humidity + "&current=" + temperature + "&desired=" + desired_temp;

  int err = 0;
  WiFiClient c;
  HttpClient http(c);

  err = http.get("52.53.170.60", 5000, url.c_str(), NULL);
  if (err == 0) {
      Serial.println("startedRequest ok");
      err = http.responseStatusCode();
      if (err >= 0) {
          // Check response code
          Serial.print("Got status code: ");
          Serial.println(err);
          err = http.skipResponseHeaders();
          if (err >= 0) {
              int bodyLen = http.contentLength();
              Serial.print("Content length is: ");
              Serial.println(bodyLen);
              Serial.println();
              Serial.println("Body returned follows:");

              // Now we've got to the body, so we can print it out
              unsigned long timeoutStart = millis();
              char c;

              // While we haven't timed out & haven't reached the end of the body
              while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
                  if (http.available()) {
                      c = http.read();
                      // Print out this character
                      Serial.print(c);
                      bodyLen--;
                      // We read something, reset the timeout counter
                      timeoutStart = millis();
                  } else {
                      // We haven't got any data, so let's pause to allow some to arrive
                      delay(kNetworkDelay);
                  }
              }
          } else {
              Serial.print("Failed to skip response headers: ");
              Serial.println(err);
          }
      } else {
          Serial.print("Getting response failed: ");
          Serial.println(err);
      }
  } else {
      Serial.print("Connect failed: ");
      Serial.println(err);
  }

  http.stop();
}

void setup() {
  // Initialization
  Serial.begin(9600);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(MOTION_SENSOR, INPUT);

  // Connect to WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nWiFi connected");


  dht.begin();

  // Initialize display on TTGO
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Display Initialized");
}

void loop() {
  // Handle button press
  if (digitalRead(BLUE_BUTTON) == LOW){
    desired_temp -= 1;
    delay(500);
  }
  else if (digitalRead(RED_BUTTON) == LOW){
    desired_temp += 1;
    delay(500);
  }

  // Read the temperature every 5 seconds BRAH
  if (millis() - lastReadTime >= readInterval) {
    float temperature = dht.readTemperature(true);
    float humidity = dht.readHumidity();

    toAWS(temperature, humidity);

    current_temp = (int)temperature;

    lastReadTime = millis();
  }

  // Change LED colors based on temperatures
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

  // Update display gauges
  if(current_temp != prev_current_temp || desired_temp != prev_desired_temp)
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


}

// Gauge design
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

// Customizes arc 
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