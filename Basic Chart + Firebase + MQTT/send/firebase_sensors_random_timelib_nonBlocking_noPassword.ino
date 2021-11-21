// ส่วนข้อมมูลที่ต้องเพิ่มเอง
// 1. WiFi
// 2. MQTT Broker
// 3. Firebase host & key

#include <ESP8266WiFi.h>    // WiFi
#include <PubSubClient.h>   // MQTT client
//#include "DHT.h"            // Temp and Humid sensor
#include <FirebaseESP8266.h>// Firebase
#include <NTPClient.h>      // Network Time Protocol - get time from network https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/
#include <TimeLib.h>    // เพื่อ convert epoch time เป็นตัวเลขที่เข้าใจได้
#include "WiFiUdp.h" 

#define WIFI_STA_NAME "ASDQWEOJJ"    //hot spot ของไวไฟ ที่แชร์จากมือถือ
#define WIFI_STA_PASS "mazikdot"

//MQTT Broker
#define MQTT_SERVER   "34.122.238.71"   //ip server ของคุณ
#define MQTT_PORT     1883
#define MQTT_USERNAME "mad"    //user mqtt ที่คุณสร้าง
#define MQTT_PASSWORD "1150"
#define MQTT_NAME     "madmqtt"   //ตั้งชื่ออะไรก็ได้

//DHT
//#define DHTPIN D1
//#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
//#define DHTTYPE DHT21 // DHT 21 (AM2301)
//DHT dht(DHTPIN, DHTTYPE);
float hif,hic;        //for cal Heat index
char temp[50];        //for convert int to char array
String temp_str = ""; //for convert int to char array

//Water sensor
int sensorPin = A0;    // select the input pin for the Soil moisture sensor
int sensorValue = 0;  // variable to store the value coming from the sensor

//Firebase เอามาจาก account ตัวเอง
#define FIREBASE_HOST "my-sensors-a3da1-default-rtdb.firebaseio.com"
#define FIREBASE_KEY "5RAXZID5WQPFyDHpoJlBPUOOuRyNXWsvfebGneEh"

FirebaseData firebaseData;
FirebaseJson json;


WiFiClient client;
PubSubClient mqtt(client);

// Define NTP Client to get time
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");
// 3600*7 เพราะประเทศไทย GMT+7 ชั่วโมง
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*7); 
unsigned long epochTime = 0;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

String MAC;
String locIp,pubIp;
int pubDelay=3000;
unsigned long time_now = 0;

void setup() {
  Serial.begin(9600);         //ปรับตามที่ตั้งค่าใน Device Manager
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);

//เซ็ตโหมดไวไฟ
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);

//พยายามเชื่อมต่อไวไฟจนกว่าจะต่อได้
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

//เชื่อมต่อสำเร็จ
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  MAC = String(WiFi.macAddress());
  locIp = WiFi.localIP().toString();
  pubIp = GetExternalIP();
  //เซ็ตค่า mqtt server
  mqtt.setServer(MQTT_SERVER, MQTT_PORT); 
  mqtt.setCallback(mqtt_callback);

  //DHT
 // dht.begin();

  //Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);

  // Initialize a NTPClient to get time
  timeClient.begin();
}

//ถ้าไวไฟหลุด จะเรียกฟังก์ชันนี้ จนกว่าจะต่อใหม่ได้
void reconnectWiFi() {  
    Serial.print("Reconnecting");
    WiFi.mode(WIFI_STA);  
    WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);  
    while (WiFi.status() != WL_CONNECTED) {  
        delay(500);  
        Serial.print(".");
    }  
    Serial.println("Connected!");
    MAC   = String(WiFi.macAddress());
    locIp = WiFi.localIP().toString();
    pubIp = GetExternalIP();
}  

//ฟังก์ชันการทำงานของ MQTT
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //อ่านข้อความที่รับมา
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  Serial.println("[" + topic_str + "]: " + payload_str);

  //กำหนดเงื่อนไขที่แตกต่างกันตาม Topic
  if( topic_str == MAC ){ //"68:C6:3A:D7:3D:E8"
  //if( topic_str == "68:C6:3A:D7:3D:E8" ){ //"68:C6:3A:D7:3D:E8"
    pubDelay = payload_str.toInt();
    Serial.print( "Publis Delay = " );
    Serial.println( payload_str );
  }if( topic_str == locIp ){ 
    Serial.print( "topic 'local ip' payload = " );
    Serial.println( payload_str );
  }if( topic_str == pubIp ){ 
    Serial.print( "topic 'public ip' payload = " );
    Serial.println( payload_str );
  }else if( topic_str == "/device001/SetT2" ){
    Serial.print( "sub-/device001/SetT2 : " );
    Serial.println( payload_str );
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

    char pubIp[17];
    int size;
    while ((size = client.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      //Serial.write(msg, size);
      int i=0,st,en;
      while(i<size){
        if(msg[i] == '{'){
          int currChIp = i+7,j=0;
          while(msg[currChIp] != '}'){
            pubIp[j]= (char)msg[currChIp];
            currChIp++;
            j++;
          }
          pubIp[j-1]='\0';
          //Serial.print("Public IP: ");
          //Serial.println(pubIp);
          str = pubIp;
          break;
        }
        i++;
      }
      //terminal.flush();
      //terminal.write(msg, size);
      //terminal.flush();
      free(msg);
    }
  }
  return str;
}

void loop() {
  
  //ถ้าการเชื่อมต่อไวไฟหาด จะพยายามต่อใหม่จนกว่าจะสำเร็จ
  if (mqtt.connected() == false) {
    Serial.print( "WiFi Status : " );
    Serial.println( WiFi.status() );
    if (WiFi.status() != WL_CONNECTED) {  
        reconnectWiFi();  
    }
    
    //พยายามเชื่อมต่อ MQTT server
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {  //for mqtt cloud
      //ต่อสำเร็จ แล้ว subscribe 2 topics
      Serial.println("connected");
      Serial.println(MAC.c_str());
      Serial.println(locIp.c_str());
      Serial.println(pubIp.c_str());
      mqtt.subscribe(MAC.c_str());
      mqtt.subscribe(locIp.c_str());
      mqtt.subscribe(pubIp.c_str());
      mqtt.subscribe("/device001/SetT2");
    } else {
      //ต่อ MQTT Server ล้มเหลว รอ 5000 mili-sec แล้วพยายามต่อใหม่
      Serial.println("failed");
      delay(5000);
    }
  } else {
    //ถ้า MQTT ต่ออยู่แล้ว ก็ให้ทำงานใน callback
    //คำสั่งนี้ไม่ควรอยู่หลัง delay 
    //เพราะจะทำให้ subscribe ไม่ได้รับข้อมูล
    mqtt.loop();
  }

  
  // wait for 'pubDelay' milli-sec with non-blocking other process
  //delay(pubDelay);
  if(millis() > time_now + pubDelay){
    //Send publish to MQTT Broker
    //mqtt.publish("/device001/hello", "Hi from Esp");
  
    /*
    //อ่านค่าจาก DHT
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }else{
      hif = dht.computeHeatIndex(f, h);
      hic = dht.computeHeatIndex(t, h, false);
      temp_str = String(h);                               //converting ftemp (the float variable above) to a string 
      temp_str.toCharArray(temp, temp_str.length() + 1);  //packaging up the data to publish to mqtt whoa...
      mqtt.publish("/device001/Humidity", temp);
      
      temp_str = String(t);                              
      temp_str.toCharArray(temp, temp_str.length() + 1);  
      mqtt.publish("/device001/TemperatureC", temp);
      
      temp_str = String(f);                              
      temp_str.toCharArray(temp, temp_str.length() + 1);  
      mqtt.publish("/device001/TemperatureH", temp);
      
      temp_str = String(hic);                              
      temp_str.toCharArray(temp, temp_str.length() + 1);  
      mqtt.publish("/device001/HeatIndexC", temp);
      
      temp_str = String(hif);                              
      temp_str.toCharArray(temp, temp_str.length() + 1);  
      mqtt.publish("/device001/HeatIndexF", temp);
    }
    
    //Water sensor
    // read the value from the sensor:
    sensorValue = analogRead(sensorPin);    
    temp_str = String(sensorValue);                              
    temp_str.toCharArray(temp, temp_str.length() + 1);  
    mqtt.publish("/device001/Water", temp);
    */
  
    //update NTP time client
    timeClient.update();
    
    //convert epoch time to date time
    /*time_t utcCalc = timeClient.getEpochTime()  ;
    String timestamp  = ctime(&utcCalc);
    String tempStr = timestamp.substring(0, timestamp.length()-1); //timestamp.c_str();
    String macStr = String(WiFi.macAddress());   
    */
    
    //create date time
    epochTime = timeClient.getEpochTime();
    String formattedTime = timeClient.getFormattedTime();
    String currentDate = String(year(epochTime)) + "-" + String(month(epochTime)) + "-" + String(day(epochTime));
    String datetime = currentDate + " " + formattedTime; 
  
    //Firebase
    json.clear().add("mac", MAC).add("datetime",datetime).add("local_ip",WiFi.localIP().toString()).add("pub_ip",GetExternalIP()).add("Humidity", String(random(20, 80))).add("TemperatureC",String(random(0, 100))).add("Water",String(random(0, 50)));
    if (Firebase.pushJSON(firebaseData, "/sensors", json)){
        Serial.println("Added"); 
    } else {
        Serial.println("Error : " + firebaseData.errorReason());
    }
  
    //update current time for waitting with non-blocking other process
    time_now = millis(); 
  } 
}
