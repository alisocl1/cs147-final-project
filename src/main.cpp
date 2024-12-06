const char* root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIEtjCCA56gAwIBAgIQCv1eRG9c89YADp5Gwibf9jANBgkqhkiG9w0BAQsFADBh\n" \
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
    "MjAeFw0yMjA0MjgwMDAwMDBaFw0zMjA0MjcyMzU5NTlaMEcxCzAJBgNVBAYTAlVT\n" \
    "MR4wHAYDVQQKExVNaWNyb3NvZnQgQ29ycG9yYXRpb24xGDAWBgNVBAMTD01TRlQg\n" \
    "UlMyNTYgQ0EtMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMiJV34o\n" \
    "eVNHI0mZGh1Rj9mdde3zSY7IhQNqAmRaTzOeRye8QsfhYFXSiMW25JddlcqaqGJ9\n" \
    "GEMcJPWBIBIEdNVYl1bB5KQOl+3m68p59Pu7npC74lJRY8F+p8PLKZAJjSkDD9Ex\n" \
    "mjHBlPcRrasgflPom3D0XB++nB1y+WLn+cB7DWLoj6qZSUDyWwnEDkkjfKee6ybx\n" \
    "SAXq7oORPe9o2BKfgi7dTKlOd7eKhotw96yIgMx7yigE3Q3ARS8m+BOFZ/mx150g\n" \
    "dKFfMcDNvSkCpxjVWnk//icrrmmEsn2xJbEuDCvtoSNvGIuCXxqhTM352HGfO2JK\n" \
    "AF/Kjf5OrPn2QpECAwEAAaOCAYIwggF+MBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
    "VR0OBBYEFAyBfpQ5X8d3on8XFnk46DWWjn+UMB8GA1UdIwQYMBaAFE4iVCAYlebj\n" \
    "buYP+vq5Eu0GF485MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcD\n" \
    "AQYIKwYBBQUHAwIwdgYIKwYBBQUHAQEEajBoMCQGCCsGAQUFBzABhhhodHRwOi8v\n" \
    "b2NzcC5kaWdpY2VydC5jb20wQAYIKwYBBQUHMAKGNGh0dHA6Ly9jYWNlcnRzLmRp\n" \
    "Z2ljZXJ0LmNvbS9EaWdpQ2VydEdsb2JhbFJvb3RHMi5jcnQwQgYDVR0fBDswOTA3\n" \
    "oDWgM4YxaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0R2xvYmFsUm9v\n" \
    "dEcyLmNybDA9BgNVHSAENjA0MAsGCWCGSAGG/WwCATAHBgVngQwBATAIBgZngQwB\n" \
    "AgEwCAYGZ4EMAQICMAgGBmeBDAECAzANBgkqhkiG9w0BAQsFAAOCAQEAdYWmf+AB\n" \
    "klEQShTbhGPQmH1c9BfnEgUFMJsNpzo9dvRj1Uek+L9WfI3kBQn97oUtf25BQsfc\n" \
    "kIIvTlE3WhA2Cg2yWLTVjH0Ny03dGsqoFYIypnuAwhOWUPHAu++vaUMcPUTUpQCb\n" \
    "eC1h4YW4CCSTYN37D2Q555wxnni0elPj9O0pymWS8gZnsfoKjvoYi/qDPZw1/TSR\n" \
    "penOgI6XjmlmPLBrk4LIw7P7PPg4uXUpCzzeybvARG/NIIkFv1eRYIbDF+bIkZbJ\n" \
    "QFdB9BjjlA4ukAg2YkOyCiB8eXTBi2APaceh3+uBLIgLk8ysy52g2U3gP7Q26Jlg\n" \
    "q/xKzj3O9hFh/g==\n" \
    "-----END CERTIFICATE-----\n";

#define SAS_TOKEN "SharedAccessSignature sr=iot147bruh.azure-devices.net%2Fdevices%2F147esp32&sig=ex7gFo6%2FKdmHcLf8j9Bv%2FABSBS%2FeZbMtsljzjyjWT8I%3D&se=1733529811"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <inttypes.h>
#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "DHT.h"
#include <TFT_eSPI.h>


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
const unsigned long readInterval = 3000;

int err = 0;
WiFiClient c;
HTTPClient http;
char ssid[50];
char pass[50];

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;

// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;


void setup() {
  Serial.begin(9600);

  strcpy(ssid, "alison phone");
  strcpy(pass, "shatfest");

  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());
  
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(MOTION_SENSOR, INPUT);

  dht.begin();

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.println("Display Initialized");
}

void loop() {
  if (digitalRead(BLUE_BUTTON) == LOW){
    desired_temp -= 1;
    delay(100);
  }
  else if (digitalRead(RED_BUTTON) == LOW){
    desired_temp += 1;
    delay(100);
  }

  // Read the temperature every readInterval seconds
  if (millis() - lastReadTime >= readInterval) {
    current_temp = dht.readTemperature(true);
    lastReadTime = millis();
    Serial.print("Current Temperature: "); Serial.print(current_temp); Serial.println(" degrees F");
    Serial.print("Desired Temperature: "); Serial.print(desired_temp); Serial.println(" degrees F");

    String current = String(current_temp, 2); // Keep 2 decimal points
    String desired = String(desired_temp, 2);

    String message = "{\"current\": " + current +
                     ", \"desired\": " + desired + "}";

    String url = "https://iot147bruh.azure-devices.net/devices/147esp32/messages/events?api-version=2016-11-14";
    http.begin(url, root_ca); //Specify the URL and certificate
    http.addHeader("Authorization", SAS_TOKEN);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(message);

    if (httpCode > 0) {
        // HTTP request was successful
        Serial.print("HTTP Code: ");
        Serial.println(httpCode);  // Success status code should be 200 or similar
        String response = http.getString();
        Serial.println("Response: " + response);
    } 
    else {
        // Something went wrong with the HTTP request
        Serial.print("HTTP request failed with code: ");
        Serial.println(httpCode);
    }
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

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.printf("Current Temp: %d F", current_temp);
  tft.setCursor(0, 50); // Move cursor to next line?
  tft.printf("Desired Temp: %d F", desired_temp);

  delay(500);

}
