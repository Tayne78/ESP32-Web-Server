

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Adafruit_I2CDevice.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "RTClib.h"
#include <ArduinoJson.h>

#define DHT_PIN 25
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

// Replace with your network credentials
const char* ssid = "ESP_AP";
const char* password = "012345678";

float temperature = 0;
float humidity = 0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


unsigned long lastTime = 0;

RTC_DS1307 rtc;

void writeFile(fs::FS &fs, const char *path, const char *message)
{
 

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}


 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);



  if (!SD.begin(5))
  {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("No SD card attached");
    return;
  }
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }
  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  writeFile(SD, "/log.txt", "Logdata:\n");


  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    
    // Read the HTML file from SPIFFS
    request->send(SPIFFS,"/index.html","text/html");
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/js");
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
    File file = SD.open("/log.txt"); // Replace with your file path
    if (file) {
      request->send(SD, "/log.txt", "text/plain");
      file.close();
    } else {
      Serial.println("ERROR loading log file!!!");
      request->send(404, "text/plain", "File not found");
    }
  });

  server.on("/api/measured-values", HTTP_GET, [](AsyncWebServerRequest *request){
        // Measure values and create a JSON response
        DynamicJsonDocument jsonDoc(256);
        jsonDoc["value1"] = temperature;  // Replace with your measured value 1
        jsonDoc["value2"] = humidity; // Replace with your measured value 2

        String response;
        serializeJson(jsonDoc, response);
        request->send(200, "application/json", response);
    });


  // Start server
  server.begin();
  dht.begin();
}
 


void loop(){

  if ((millis() - lastTime) > 1000)
  {
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    Serial.println(temperature);
    Serial.println(humidity);
    DateTime now = rtc.now();
    String data = String(now.year()) + "." + String(now.month()) + "." + String(now.day()) + "/" + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + " Data: T = " + String(temperature) + " H = " + String(humidity) + "\n";

    writeFile(SD, "/log.txt", data.c_str());
    lastTime = millis();
  }
}