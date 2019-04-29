#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>

#include <SoftwareSerial.h> 

#include "Adafruit_VL53L0X.h"


#define HOSTNAME "ESP8266-OTA-"
#define HTTP_REST_PORT 80

#define SDA    D6
#define SCL    D5
 
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

unsigned int mmDist = 0;

// Replace with your network details

void configModeCallback (WiFiManager *myWiFiManager);
void getSensor();

float distance = 0;
String welcomeStr = "";

// Web Server on port 80
ESP8266WebServer http_rest_server(HTTP_REST_PORT);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  delay(10);

  // LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

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
  
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  config_rest_server_routing();
  http_rest_server.begin();
  Serial.println("HTTP REST Server Started");

  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
}


// runs over and over again
void loop() {
  http_rest_server.handleClient();
} 


void getSensor() {
    // Active LED
    digitalWrite(LED_BUILTIN, LOW);
    
    VL53L0X_RangingMeasurementData_t measure;
    int valid = 1;
    int try_times = 100;

    int n = 0;
    while (n < try_times && valid == 1) // Try try_times to get a relative accuate value
    {
      n += 1;
      lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

      if (measure.RangeStatus != 4) {  // phase failures have incorrect data
          distance += measure.RangeMilliMeter;
          Serial.println(measure.RangeMilliMeter);
          delay(1);
      } else {
        distance = -1;
        valid = 0;
      }
    }

    if (valid != 0) {
      distance = int(distance / try_times);
    }

    Serial.println("distance:");
    Serial.println(distance);

    DynamicJsonDocument jsonObj(1024);

    char JSONmessageBuffer[200];

    jsonObj["distance"] = distance;
    serializeJsonPretty(jsonObj, JSONmessageBuffer);
    //serializeJsonPretty
    Serial.println(JSONmessageBuffer);

    http_rest_server.send(200, "application/json", JSONmessageBuffer);
    // Disable LED
    digitalWrite(LED_BUILTIN, HIGH); 
}

void config_rest_server_routing() {
    char *vl53l0x_api = "/vl53l0x";
    welcomeStr += "<p>Welcome to the ESP8266 Sensor Server</p>";
    welcomeStr += "<p>";
    welcomeStr += "API: http://";
    welcomeStr += WiFi.localIP().toString();
    welcomeStr += vl53l0x_api;
    welcomeStr += "</p>";

    Serial.println(welcomeStr);
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            welcomeStr);
    });
    http_rest_server.on("/vl53l0x", HTTP_GET, getSensor);
    // http_rest_server.on("/leds", HTTP_POST, post_put_leds);
    // http_rest_server.on("/leds", HTTP_PUT, post_put_leds);
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
