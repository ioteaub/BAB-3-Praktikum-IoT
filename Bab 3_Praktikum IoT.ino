#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#include <DHT.h>

#define DHT11PIN 15
#define LED_REDPIN 13
#define LED_YELLOWPIN 12
#define LED_ONOFF 14
#define DHTTYPE DHT11
DHT dht(DHT11PIN,DHTTYPE);

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Praktikum IoT"
#define WIFI_PASSWORD "InternetofThings"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCTdYV44uVge6S4tuEyeNL8jtZMY792pMo "

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://praktikum-iot-9c140-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

String intLED1;

unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;

void setup(){
  Serial.begin(9600);
  dht.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(LED_REDPIN,OUTPUT);
  pinMode(LED_YELLOWPIN,OUTPUT);
  pinMode(LED_ONOFF,OUTPUT);

}

void loop(){

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  Serial.print("Humidity (%): ");
  Serial.println(h);
  if (h>=80)
  {
    digitalWrite(LED_REDPIN, HIGH);
  }
  else 
  {
    digitalWrite(LED_REDPIN, LOW);
  }

  Serial.print("Temperature  (C): ");
  Serial.println(t);
    if (t>=31)
  {
    digitalWrite(LED_YELLOWPIN, HIGH);
  }
  else 
  {
    digitalWrite(LED_YELLOWPIN, LOW);
  }

  delay(2000);
  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int (suhu)
    if (Firebase.RTDB.setInt(&fbdo, "Data/Suhu", t)){
       Serial.println("PASSED");
       Serial.println("PATH: " + fbdo.dataPath());
       Serial.println("TYPE: " + fbdo.dataType());
     }
     else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
     }

     // Write an Int number on the database path test/int (kelembapan)
     if (Firebase.RTDB.setInt(&fbdo, "Data/Kelembapan", h)){
       Serial.println("PASSED");
       Serial.println("PATH: " + fbdo.dataPath());
       Serial.println("TYPE: " + fbdo.dataType());
     }
     else {
       Serial.println("FAILED");
       Serial.println("REASON: " + fbdo.errorReason());
     }

    //led 1
    if (Firebase.RTDB.getString(&fbdo, "Data/LED")) {
      if (fbdo.dataType() == "string") {
        intLED1 = fbdo.stringData();
        if(intLED1 == "1"){
          digitalWrite(LED_ONOFF, HIGH);
          }
        }
        if(intLED1 == "0"){
          digitalWrite(LED_ONOFF, LOW);
        }
        Serial.println(intLED1);
    }
    else {
      Serial.println(fbdo.errorReason());
    }
  }
}