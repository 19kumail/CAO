//Including all necessary Libraries
#include <Arduino.h>  //Necessary when using platformio
#include <WiFi.h> //Connect ESP32 to the internet
#include <Firebase_ESP_Client.h>  //Interface the board with Firebase
#include <Wire.h> //To interface with the DHT
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "time.h" //To get time

//To ensure proper working of Firebase library
#include "addons/TokenHelper.h" // Provide the token generation process info.
#include "addons/RTDBHelper.h"  // Provide the RTDB payload printing info and other helper functions.

//Initializing credentials for Wifi Network
const char* ssid = "Kumail";
const char* password = "lmaoscenes";

//Firebase Project API key
#define API "AIzaSyAmwgK1hdavhPPIhJxSjvEBIkYaa5g1gO8"

//Initializing credentials for Firebase
#define FB_EMAIL "shaider.bscs20seecs@seecs.edu.pk"
#define FB_PASS   "seecsisthebest"

//Database URL
#define DB_URL "https://home-automation-f5bdf-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Firebase objects
FirebaseData fbd;
FirebaseAuth fba;
FirebaseConfig fbc;

String uid; //Save UID of USER

String dbpath;  //Database main path

//Database children nodes
String temp = "/Temperature";
String hum = "/Humidity";
String infra = "/Infrared";
String gas = "/Gas";
String Time = "/Time";

String parent;  //Parent node updated in every loop
int timestamp;

FirebaseJson json;  //Variable of FirebaseJson
const char* ntpserver = "pool.ntp.org";  //To request time from a cluster of time severs
//DHT dht; //Sensor object created on ESP32 default I2C

//Variables to store sensor readings
float temperature;
float humidity;
float infrared;
float gastype;

//Time variable (readings after every second)
unsigned long prevtime = 0;
unsigned long delaytime = 1000;

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

unsigned long getTime(){
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    return(0);
  }
  time(&now);
  return now;
}

DHT dht(23,DHT11);
void setup(){
    Serial.begin(115200);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
    connWiFi(); //Call Function
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
    Serial.println(F("DHTxx test!"));
    dht.begin();
    configTime(0,0,ntpserver);

    fbc.api_key = API;   // Assign the api key (required)

    // Assign the user sign in credentials
    fba.user.email = FB_EMAIL;
    fba.user.password = FB_PASS;

    // Assign the RTDB URL (required)
    fbc.database_url = DB_URL;

    Firebase.reconnectWiFi(true);
    fbd.setResponseSize(4096);

    // Assign the callback function for the long running token generation task */
    fbc.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    // Assign the maximum retry of token generation
    fbc.max_token_generation_retry = 5;

    // Initialize the library with the Firebase authen and config
    Firebase.begin(&fbc, &fba);

    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((fba.token.uid) == "") {
      Serial.print('.');
      delay(1000);
    }
    // Print user UID
    uid = fba.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

    // Update database path
    dbpath = "/UsersData/" + uid + "/readings";
}

void loop(){
  if (WiFi.status()!= WL_CONNECTED){
    Serial.println("You are now Offline.");
    WiFi.disconnect();
    connWiFi();
  }
  delay(1000);
  // Send new readings to database
  if (Firebase.ready() && (millis() - prevtime > delaytime || prevtime == 0)){
    prevtime = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parent= dbpath + "/" + String(timestamp);

    json.set(temp.c_str(), String(dht.readTemperature()));
    json.set(hum.c_str(), String(dht.readHumidity()));
    //json.set(presPath.c_str(), String(bme.readPressure()/100.0F));
    json.set(Time, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbd, parent.c_str(), &json) ? "ok" : fbd.errorReason().c_str());
  }
}