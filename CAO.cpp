#include <Arduino.h>  //Necessary when using platformio
#include <WiFi.h> //Connect ESP32 to the internet
#include <Wire.h> //To interface with the DHT
#include <Firebase_ESP_Client.h>  //Interface the board with Firebase
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <MQUnifiedsensor.h>
#include <LiquidCrystal_I2C.h>
#include "time.h" //To get time

//To ensure proper working of Firebase library
#include "addons/TokenHelper.h" // Provide the token generation process info.
#include "addons/RTDBHelper.h"  // Provide the RTDB payload printing info and other helper functions.

//Initializing credentials for Wifi Network
#define WIFI_SSID "Kumail"
#define WIFI_PASSWORD "kumail123"

//Firebase Project API key
#define API "AIzaSyAmwgK1hdavhPPIhJxSjvEBIkYaa5g1gO8"

//Initializing credentials for Firebase
#define FB_EMAIL "shaider.bscs20seecs@seecs.edu.pk"
#define FB_PASS   "seecsisthebest"

//Database URL
#define DB_URL "https://home-automation-f5bdf-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid; //Save UID of USER

String databasePath;  //Database main path

//Database children nodes
String tempPath = "/temperature";
String humPath = "/humidity";
String infraPath = "/infrared";
String gasPath = "/gas";
String timePath = "/timestamp";

String parentPath;  //Parent node updated in every loop
int timestamp;
FirebaseJson json;  //Variable of FirebaseJson
const char* ntpserver = "pool.ntp.org";  //To request time from a cluster of time severs
//DHT dht; //Sensor object created on ESP32 default I2C

//Variables to store sensor readings
float temperature;
float humidity;
float infrared;
float gas;

//Time variable (readings after every 10 seconds)
unsigned long prevtime = 0;
unsigned long delaytime = 1000;
// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

DHT dht(23,DHT11);
MQUnifiedsensor MQ4("Esp32 Dev Module", 3.3, 12, 34, "MQ-4");

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
String Connect = "Connecting to WiFi ......" ;
String Offline = "You are now Offline.";

//DHT dht(2,DHT11);
//MQUnifiedsensor MQ4("Esp32 Dev Module", 3.3, 10, A0, "MQ-4");
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

void connWiFi() { //Function for connection to Wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println("You are now Online.");
}

unsigned long getTime() { // Function that gets current epoch time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  now = now++;
  time(&now);
  return now;
}
const int extinguish = 25;
const int fan = 33;
const int exhaust = 32;
const int cooler = 26;
const int buzz = 18;
void setup(){
    Serial.begin(115200);
    //Serial.begin(9600);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
    connWiFi(); //Call Function
    Serial.print("RRSI: ");
    Serial.println(WiFi.RSSI());
    Serial.println(F("DHTxx test!"));
  
    lcd.init();    // initialize LCD                   
    lcd.backlight();  // turn on LCD backlight
    dht.begin();
    MQ4.init();
    pinMode (extinguish, OUTPUT);
    pinMode (fan, OUTPUT);
    pinMode (exhaust, OUTPUT);
    pinMode (cooler, OUTPUT);
    pinMode (buzz, OUTPUT);

    configTime(0,0,ntpserver);
     // Assign the api key (required)
  config.api_key = API;

  // Assign the user sign in credentials
  auth.user.email = FB_EMAIL;
  auth.user.password = FB_PASS;

  // Assign the RTDB URL (required)
  config.database_url = DB_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

    // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

String ALERT1 = "FIRE ALERT!!!";
String ALERT2 = "TOO HOT AND DRY!!!";
String ALERT3 = "GAS LEAK!!!";   
String ALERT4 = "TOO HOT AND HUMID!!!";
String SOL1 = "FIRE NEUTRALISED!!!";
String SOL2 = "FAN TURNED ON!!!";
String SOL3 = "EXHAUST TURNED ON!!!";
String SOL4 = "COOLER TURNED ON!!!";
String FINE = "EVERYTHING OK!!!";

void loop(){
  lcd.setCursor(0, 0);
  if (WiFi.status()!= WL_CONNECTED){
    Serial.println("You are now Offline.");
    scrollText(1,Offline,250,lcdColumns);
    WiFi.disconnect();
  }
  lcd.clear();
  // Send new readings to database
  if (Firebase.ready() && (millis() - prevtime > delaytime || prevtime == 0)){
    prevtime = millis();

    //Get current timestamp
    timestamp = getTime();
    Serial.print ("time: ");
    Serial.println (timestamp);

    parentPath= databasePath + "/" + String(timestamp);

    json.set(tempPath.c_str(), String(dht.readTemperature()));
    json.set(humPath.c_str(), String(dht.readHumidity()));
    json.set(gasPath.c_str(), String(analogRead(34)));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());  }

  if(dht.readTemperature()>35 && analogRead(34)>750){
    Serial.print("FIRE ALERT!!!");
    digitalWrite(buzz,HIGH);
    digitalWrite (extinguish, HIGH);	// turn on the LED
    scrollText(1,ALERT1,250,lcdColumns);
    lcd.clear();
    delay(3000);
    scrollText(1,SOL1,250,lcdColumns);
    digitalWrite(buzz,LOW);
    digitalWrite (extinguish, LOW);
  }
  else if(analogRead(34)>750){
    Serial.print("GAS LEAK!!!");
    digitalWrite(buzz,HIGH);
    digitalWrite (exhaust, HIGH);
    scrollText(1,ALERT3,250,lcdColumns);
    lcd.clear();
    delay(3000);
    scrollText(1,SOL3,250,lcdColumns);
    digitalWrite(buzz,LOW);
    digitalWrite (exhaust, LOW);    
  }
  else if(dht.readTemperature()>33 && dht.readHumidity()>40){
    Serial.print("HOT AND HUMID!!!");
    digitalWrite (fan, HIGH);
    scrollText(1,ALERT4,250,lcdColumns);
    lcd.clear();
    delay(3000);
    scrollText(1,SOL2,250,lcdColumns);
    digitalWrite (fan, LOW);
  }
  else if(dht.readTemperature()>33 && dht.readHumidity()<=40){
    Serial.print("HOT AND DRY!!!");
    digitalWrite (fan, HIGH);
    digitalWrite (cooler, HIGH);
    scrollText(1,ALERT2,250,lcdColumns);
    lcd.clear();
    delay(3000);
    scrollText(1,SOL2,250,lcdColumns);
    lcd.clear();
    scrollText(1,SOL4,250,lcdColumns);
    digitalWrite (fan, LOW);
    digitalWrite (cooler, LOW);
  }
  else{
    Serial.print("NORMAL!!!");
    scrollText(1,FINE,250,lcdColumns);
    lcd.clear();
  }
 
}