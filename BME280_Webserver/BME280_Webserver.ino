
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
#include <ArduinoJson.h>

#define HOSTNAME "ESP8266-OTA-"
#define SEALEVELPRESSURE_HPA (1013.25)
#define HTTP_REST_PORT 80

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
void config_rest_server_routing();


float temperature, humidity, pressure, altitude;
String welcomeStr = "";

// Web Server on port 80
ESP8266WebServer http_rest_server(HTTP_REST_PORT);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);

  bme.begin(0x76);
  Wire.begin(D6, D5);
  Wire.setClock(100000);

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
  // WiFi.hostname(hostname);
  WiFi.hostname("BME280");

  
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

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Starting the web server
  config_rest_server_routing();
  http_rest_server.begin();
  Serial.println("HTTP REST Server Started");
}


// runs over and over again
void loop() {
    http_rest_server.handleClient();
} 




void getSensor() {
    // Active LED
    digitalWrite(LED_BUILTIN, LOW);
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    Serial.println("temperature:");
    Serial.println(temperature);
    Serial.println("humidity:");
    Serial.println(humidity);
    Serial.println("pressure:");
    Serial.println(pressure);

    DynamicJsonDocument jsonObj(1024);

    char JSONmessageBuffer[200];

    jsonObj["temperature"] = temperature;
    jsonObj["humidity"] = humidity;
    jsonObj["pressure"] = pressure;
    serializeJsonPretty(jsonObj, JSONmessageBuffer);
    //serializeJsonPretty
    Serial.println(JSONmessageBuffer);

    http_rest_server.send(200, "application/json", JSONmessageBuffer);
    // Disable LED
    digitalWrite(LED_BUILTIN, HIGH);
}

void config_rest_server_routing() {
    char *api_string = "/bme280";
    String api_link = "http://" + WiFi.localIP().toString() + api_string;
    welcomeStr += "<p>Welcome to the ESP8266 Sensor Server</p>";
    welcomeStr += "<p>";
    welcomeStr += "API: ";
    welcomeStr += "<a href=\"" + api_link + "\">" + api_link + "</a>";
    welcomeStr += "</p>";

    Serial.println(welcomeStr);
    http_rest_server.on("/", HTTP_GET, []() {
        http_rest_server.send(200, "text/html",
            welcomeStr);
    });
    http_rest_server.on(api_string, HTTP_GET, getSensor);
    // http_rest_server.on("/leds", HTTP_POST, post_put_leds);
    // http_rest_server.on("/leds", HTTP_PUT, post_put_leds);
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
