#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID "ASDQWEOJJ" 
#define WIFI_PASSWORD "mazikdot"

#define FIREBASE_HOST "my-sensors-a3da1-default-rtdb.firebaseio.com"
#define FIREBASE_KEY "5RAXZID5WQPFyDHpoJlBPUOOuRyNXWsvfebGneEh"
FirebaseData firebaseData;
bool state = false;
//#define TRIGGER_PIN  D1
//#define ECHO_PIN     D2
#include "DHT.h"
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#define DHTPIN 2     
#define DHTTYPE DHT22   
unsigned long period = 2000; 
unsigned long last_time_light = 0; 
unsigned long last_time = 0; 
int analogPin = A0;
DHT dht(DHTPIN, DHTTYPE);
//NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 7);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 7);
unsigned long epochTime = 0;
float h,t,f,hif,hic;
int LED_LUK = D7;
int LED_GAS = D8;
int val = 0;
uint16_t lux = 0;
bool StateGas = false;
unsigned long time_ledGas;
#include <BH1750FVI.h>
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
void setup() {
    connectWifi();
    Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
    pinMode(LED_LUK, OUTPUT);
    pinMode(LED_GAS, OUTPUT);
    dht.begin();
    LightSensor.begin();
}

void loop() {
//------------Start time------------------------------------
  val = analogRead(analogPin);
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  String Time = timeClient.getFormattedTime();
  String date = String(year(epochTime)) + "-"  + String(month(epochTime)) + "-" + String(day(epochTime));
  String date_time = date + " " + Time;
  ledGas();
   if(lux < 10 ){
      
      digitalWrite(LED_LUK,1);
    }
    else
    {
      digitalWrite(LED_LUK,0);
    }
    if (val > 200) { // สามารถกำหนดปรับค่าได้ตามสถานที่ต่างๆ
    
    StateGas = true;
    
  }
  else {
     digitalWrite(LED_GAS, 0); // สั่งให้ LED ดับ
     StateGas = false;
  }
 
  //Serial.println(date_time);
//  ------------End time-------------------
   if( millis() - last_time > period) {
    
  last_time = millis(); //เซฟเวลาปัจจุบันไว้เพื่อรอจนกว่า millis() จะมากกว่าตัวมันเท่า period 
   h = dht.readHumidity();
   t = dht.readTemperature();
   f = dht.readTemperature(true);
   hif = dht.computeHeatIndex(f, h);
   hic = dht.computeHeatIndex(t, h, false);
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F(" C "));
  Serial.print(f);
  Serial.print(F(" F  Heat index: "));
  Serial.print(hic);
  Serial.print(F(" C "));
  Serial.print(hif);
  Serial.println(F(" F"));
  Serial.println(date_time);
//  -------------------Firebase------------------------------
  FirebaseJson data,dataGAS,dataLight;
  data.set("Humiduty ", String(h));
  data.set("TemperatureC", String(t));
  data.set("Temperature : F ", String(f));
  data.set("Heat : index : C", String(hic)); 
  data.set("Heat : index : F", String(hif));
  data.set("datetime", String(date_time));
  dataLight.set("light",String(lux));
  dataLight.set("datetime",String(date_time));
  dataGAS.set("gas",String(val));
  dataGAS.set("datetime",String(date_time));
  if ((Firebase.pushJSON(firebaseData, "/sensorProject", data)) && (Firebase.pushJSON(firebaseData, "/GAS", dataGAS)) && (Firebase.pushJSON(firebaseData, "/Light", dataLight))) {
    Serial.println("Pushed : " + firebaseData.pushName());
  } else {
    Serial.println("Error : " + firebaseData.errorReason());
  }
//  if((Firebase.setFloat(firebaseData, "/dataShow/Humiduty", h))     &&  (Firebase.setFloat(firebaseData, "/dataShow/TemperatureC", t)) && (Firebase.setInt(firebaseData, "/dataShow/Light", lux)) && (Firebase.setInt(firebaseData, "/dataShow/Gas", val))  ) {
//        Serial.println("Added"); 
//    } else {
//        Serial.println("Error : " + firebaseData.errorReason());
//    }
//  
 }
//  -------------------- End Firebase--------------------------------
    if( millis() - last_time_light > 1000) {
     last_time_light = millis(); 
     lux = LightSensor.GetLightIntensity();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lux");
    Serial.print("val = "); // พิมพ์ข้อมความส่งเข้าคอมพิวเตอร์ "val = "
    Serial.println(val); // พิมพ์ค่าของตัวแปร val
   
 }
 
}

void connectWifi() {
    Serial.begin(115200);
    Serial.println(WiFi.localIP());
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
}
void ledGas(){
   if( millis() - time_ledGas > 100 && StateGas == true) {
     time_ledGas = millis(); 
     Serial.println("แก๊สเกิน สัญญาณเตือนดัง !!!");
     digitalWrite(LED_GAS,!digitalRead(LED_GAS));
 }
}
