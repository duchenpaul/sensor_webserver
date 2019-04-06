
/*********
  Project: BME Weather Web server using NodeMCU
  Implements Adafruit's sensor libraries.
  Complete project is at: http://embedded-lab.com/blog/making-a-simple-weather-web-server-using-esp8266-and-bme280/
  
  Modified code from Rui Santos' Temperature Weather Server posted on http://randomnerdtutorials.com  
*********/

#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define HOSTNAME "ESP8266-OTA-"
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C
/*
#define D1 5
#define D2 4
#define D4 2
#define D3 0
*/

// Replace with your network details
// const char* ssid = "Wolfstein";
// const char* password = "++++----";

void configModeCallback (WiFiManager *myWiFiManager);
void getSensor();


float temperature, humidity, pressure, altitude;


// Web Server on port 80
WiFiServer server(80);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  bme.begin(0x76);
  Wire.begin(D6, D5);
  Wire.setClock(100000);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  // Uncomment for testing wifi manager
  // wifiManager.resetSettings();
  wifiManager.setAPCallback(configModeCallback);

  //or use this for auto generated name ESP + ChipID
  wifiManager.autoConnect();

  //Manual Wifi
  //WiFi.begin(WIFI_SSID, WIFI_PWD);
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  
  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  
  // WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Starting the web server
  server.begin();
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  Serial.println(F("BME280 service started"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}


// runs over and over again
void loop() {
  // Listenning for new clients
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.readStringUntil('>')) {
        char c = client.read();
        
        getSensor();
        String ptr = "";
        ptr += "{\"Temperature\":";
        ptr += temperature;
        ptr += ",\"Humidity\":";
        ptr += humidity;
        ptr += ",\"Pressure\":";
        ptr += pressure;
        // ptr += ",\"Altitude\":";
        // ptr += altitude;
        ptr += "}";
        Serial.println(ptr);
        client.println(ptr);
        delay(100);
        break;
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
} 




void getSensor() {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA); 
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
