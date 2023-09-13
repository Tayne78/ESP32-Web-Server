

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <Adafruit_I2CDevice.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

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


 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);


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
    String sensorData;
    File file = SPIFFS.open("/index.html", "r");
    if (file) {
        sensorData = file.readString();
        file.close();

        // Replace placeholders with sensor data
        sensorData.replace("{{temperature}}", String(temperature));
        sensorData.replace("{{humidity}}", String(humidity));

        // Send the modified HTML to the client
        request->send(200, "text/html", sensorData);
    } else {
        request->send(404, "text/plain", "File not found");
    }
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });


  // Start server
  server.begin();
  dht.begin();
}
 
void loop(){
  delay(2000);

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  Serial.println(temperature);
  Serial.println(humidity);
}