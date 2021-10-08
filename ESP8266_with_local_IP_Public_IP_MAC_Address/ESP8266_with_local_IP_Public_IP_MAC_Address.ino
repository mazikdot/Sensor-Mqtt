#include <ESP8266WiFi.h>

#define WIFI_SSID "ASDQWEOJJ"
#define WIFI_PASSWORD "mazikdot"

String text;

void setup() {
  connectWifi();
  text = GetExternalIP();
}

void loop() {
  delay(3000);
  Serial.print("Local IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Public IP: ");
  Serial.println(text);
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("----------------------------");
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
  Serial.println("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}
