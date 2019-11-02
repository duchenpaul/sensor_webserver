#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>




#define HOSTNAME "ESP8266-OTA-"

int heart_beat_ind = 10;

void configModeCallback (WiFiManager *myWiFiManager);

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}

void setup()
{
    Serial.begin(115200);
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
    WiFi.hostname("flamesensor_client");


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

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    // WiFi.begin(ssid, password);

}

void loop()
{
    heart_beat_ind += 1;

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http; //Object of class HTTPClient
        http.begin("http://192.168.31.139/flamesensor");
        int httpCode = http.GET();

        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println(payload);

            // Allocate JsonBuffer
            // Use arduinojson.org/assistant to compute the capacity.
            DynamicJsonDocument root(1024);
            deserializeJson(root, payload);

            boolean flameStatus = root["flame_status"].as<boolean>();

            if (flameStatus) {
                digitalWrite(LED_BUILTIN, LOW);
            }
            else {
                digitalWrite(LED_BUILTIN, HIGH);
                if ( heart_beat_ind % 3 == 0 ) {
                    digitalWrite(LED_BUILTIN, LOW);
                    delay(200);
                    digitalWrite(LED_BUILTIN, HIGH);
                    delay(500);
                }
            }


        }
        else {
            Serial.println("Sensor not found.");
            digitalWrite(LED_BUILTIN, HIGH);
        }
        http.end(); //Close connection
    }
    delay(100);
}