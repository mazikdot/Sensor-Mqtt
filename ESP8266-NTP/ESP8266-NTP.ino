#include <NTPClient.h>  // ดึงเวลาปัจจุบันจาก internet (epoch time)
#include <TimeLib.h>    // เพื่อ convert epoch time เป็นตัวเลขที่เข้าใจได้
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char *ssid     = "ASDQWEOJJ";
const char *password = "mazikdot";

WiFiUDP ntpUDP;

// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
//NTPClient timeClient(ntpUDP);
// 3600*7 เพราะประเทศไทย GMT+7 ชั่วโมง
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*7); 
unsigned long epochTime = 0;

// You can specify the time server pool and the offset, (in seconds)
// additionally you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
}

void loop() {
  
  timeClient.update();
  epochTime = timeClient.getEpochTime();
  Serial.print("day: ");    Serial.println(day(epochTime));
  Serial.print("month: ");  Serial.println(month(epochTime));
  Serial.print("year: ");   Serial.println(year(epochTime));
  Serial.print("h: ");      Serial.println(hour(epochTime));
  Serial.print("m: ");      Serial.println(minute(epochTime));
  Serial.print("s: ");      Serial.println(second(epochTime));
  
  Serial.println(timeClient.getFormattedTime());
  
  delay(1000);
}
