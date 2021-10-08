#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define WIFI_SSID "ASDQWEOJJ" 
#define WIFI_PASSWORD "mazikdot"

#define FIREBASE_HOST "my-sensors-a3da1-default-rtdb.firebaseio.com"
#define FIREBASE_KEY "5RAXZID5WQPFyDHpoJlBPUOOuRyNXWsvfebGneEh"
FirebaseData firebaseData;
bool state = false;
#define TRIGGER_PIN  D1
#define ECHO_PIN     D2
void setup() {
    connectWifi();
    Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
    // Do something
     pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
}

void loop() {
    // Do something
   int duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;
  Serial.print(distance);
  Serial.println(" cm");
  delay(1000);
    //delay(1000);
    //state = !state;
    if(Firebase.setInt(firebaseData, "/mySensor/distance", distance)) {
        Serial.println("Added"); 
    } else {
        Serial.println("Error : " + firebaseData.errorReason());
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
