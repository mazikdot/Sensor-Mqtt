#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>   // MQTT client

#define WIFI_SSID "ASDQWEOJJ"
#define WIFI_PASSWORD "mazikdot"

#define FIREBASE_HOST "my-sensors-a3da1-default-rtdb.firebaseio.com"
#define FIREBASE_KEY "5RAXZID5WQPFyDHpoJlBPUOOuRyNXWsvfebGneEh"

//MQTT protocol
#define MQTT_SERVER   "34.122.238.71"   //ip server ของคุณ
#define MQTT_PORT     1883
#define MQTT_USERNAME "mad"    //user mqtt ที่คุณสร้าง
#define MQTT_PASSWORD "1150"
#define MQTT_NAME     "madmqtt"   //ตั้งชื่ออะไรก็ได้

FirebaseData firebaseData;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600 * 7);
unsigned long epochTime = 0;
String pubIP, locIP, mac_addr;

WiFiClient client;
PubSubClient mqtt(client);


int textDelay;

void setup() {
  connectWifi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
  timeClient.begin();
  pubIP = GetExternalIP();
  locIP = WiFi.localIP().toString() ;
  mac_addr = WiFi.macAddress();
  
  //เซ็ตค่า mqtt server
  WiFiClient client;
  PubSubClient mqtt(client);
}

//ฟังก์ชันการทำงานของ MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //อ่านข้อความที่รับมา
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  //กำหนดเงื่อนไขที่แตกต่างกันตาม Topic
  if ( topic_str == "/device001/SetT1" ) {
    Serial.print( "delay : " );
    Serial.println( payload_str );
  } else if ( topic_str == "/device001/SetT2" ) {
    Serial.print( "delay : " );
    Serial.println( payload_str );
  }
  textDelay = payload_str.toInt(); 
} 

void loop() {
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  String Time = timeClient.getFormattedTime();
  String date = String(year(epochTime)) + "-"  + String(month(epochTime)) + "-" + String(day(epochTime));
  String date_time = date + " " + Time;
  delay(textDelay); //set delay 
  int Humiduty, TemperatureC, Water;
  Humiduty = random(50);
  TemperatureC = random(50);
  Water = random(50);

  FirebaseJson data;
  data.set("Humiduty ", Humiduty);
  data.set("TemperatureC ", TemperatureC);
  data.set("Water ", Water);
  data.set("datetime ", date_time);
  data.set("local_ip ", locIP);
  data.set("mac_ip ", mac_addr);
  data.set("pub_ip ", pubIP);

  delay(3000);
  if (Firebase.pushJSON(firebaseData, "/sensor", data)) {
    Serial.println("Pushed : " + firebaseData.pushName());
  } else {
    Serial.println("Error : " + firebaseData.errorReason());
  }
}

String GetExternalIP()
{
  String str = "";
  WiFiClient client;
  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("Failed to connect with 'api.ipify.org' !");
  }
  else
  {
    int timeout = millis() + 5000;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");


    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return str;
      }
    }

    char pubIp[17], pubIp_s, pubIp_e;
    int size;
    while ((size = client.available()) > 7) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      Serial.write(msg, size);
      int i = 0, st, en;
      while (i < size) {
        if (msg[i] == '{') {
          int currChIp = i + 7, j = 0;
          pubIp_s = (char)msg[currChIp];
          while (msg[currChIp] != '}') {
            pubIp_e = (char)msg[currChIp - 1];
            pubIp[j] = (char)msg[currChIp];
            currChIp++;
            j++;
          }
          pubIp[j - 1] = '\0';
          Serial.print("Public IP Start with: ");
          Serial.println(pubIp_s);
          Serial.print("Public IP End with: ");
          Serial.println(pubIp_e);
          str = pubIp;
          break;
        }
        i++;
      }
      free(msg);
    }
  }
  return str;
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
