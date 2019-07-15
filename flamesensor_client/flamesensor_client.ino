#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "ssid";
const char* password = "password";

int heart_beat_ind = 10;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
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

            boolean flameStatus = root["flame status"].as<boolean>();

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