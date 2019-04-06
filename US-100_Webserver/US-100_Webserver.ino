#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)

#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


#include <SoftwareSerial.h> 

#define HOSTNAME "ESP8266-OTA-"

const int US100_TX = D8;
const int US100_RX = D7;
 
// Instancia nuevo canal serie
SoftwareSerial US100(US100_RX, US100_TX);

unsigned int MSByteDist = 0;
unsigned int LSByteDist = 0;
unsigned int mmDist = 0;
int temp = 0;

// Replace with your network details

void configModeCallback (WiFiManager *myWiFiManager);
void getSensor();

float temperature, distance;

// Web Server on port 80
WiFiServer server(80);

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  US100.begin(9600);
  delay(10);

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
  Serial.println(F("US-100 service started"));

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
      Serial.println("Client connected");
      // if (client.available()) {
      if (client.readStringUntil('>')) {
        Serial.println("Client available");
        char c = client.read();
        Serial.println("Client read");
        
        // if (c == '\n' && blank_line) {
        if (1 == 1) {
            delay(100);
            readSensor();
            String ptr = "";
            ptr += "{\"distance\":";
            ptr += distance;
            ptr += ",\"Temperature\":";
            ptr += temperature;
            ptr += "}";
            Serial.println(ptr);
            client.println(ptr);
            delay(100);
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
      else {
        Serial.println("Client NOT available, re-try");
        
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
} 




void readSensor() {

    US100.flush(); 
    US100.write(0x55); // Read distance

    delay(100);

    if(US100.available() >= 2)
    {
        MSByteDist = US100.read();
        LSByteDist  = US100.read();
        mmDist  = MSByteDist * 256 + LSByteDist;
        if((mmDist > 1) && (mmDist < 10000))
        {
            Serial.print("Distance: ");
            Serial.print(mmDist, DEC);
            Serial.println(" mm");
            distance = mmDist;
        }
    }


    US100.flush(); 
    US100.write(0x50); // Read temperature

    delay(100);

    if(US100.available() >= 1)
    {
        temp = US100.read();
        if((temp > 1) && (temp < 130))
        {
            temp -= 45;
            Serial.print("Temperature: ");
            Serial.print(temp, DEC);
            Serial.println(" ºC.");
            temperature = temp;
        }
    }
 
}


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

}
