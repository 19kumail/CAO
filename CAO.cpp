#include <Arduino.h>

//Including Wifi Library
#include "WiFi.h"
//Initializing credentials for Wifi Network
const char* ssid = "Kumail";
const char* password = "lmaoscenes";

void connWiFi() { //Function for connection to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); //Passing Credentials
  Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
    }
  Serial.println(WiFi.localIP());
  Serial.println("You are now Online.");

}
void setup()
{
    Serial.begin(115200);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
    connWiFi(); //Call Function
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
}
void loop()
{
  if (WiFi.status()!= WL_CONNECTED){
    Serial.println("You are now Offline.");
    WiFi.disconnect();
    connWiFi();
  }
  delay(1000);
    /*Serial.println("scan start");
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
     
    }
    Serial.println("");
    // Wait a bit before scanning again
    delay(1000);*/  
}