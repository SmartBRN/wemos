#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

#define DHTTYPE DHT22
#define PIN_DHT D3

#define PIN_LIGHT_SWITCH D7

#define SWITCH_LIGHT_URL "/light"

const char* ssid = "DIR-620";
const char* password = "76543210";

bool lightState = false;

WiFiServer server(80);

void setup() {
 Serial.begin(9600);
 while(!Serial) {
  continue;
 }
 Serial.println("Serial port is ready.");

 Serial.println("Serial port is ready.");

 WiFi.mode(WIFI_STA);
 WiFi.begin(ssid, password);
 while(WiFi.waitForConnectResult() != WL_CONNECTED) {
  Serial.println("WiFi connection failed. Retry...");
  delay(10000);
  ESP.restart();
 }

 Serial.print("WiFi is ready. Local ip: ");
 Serial.println(WiFi.localIP());

 server.begin();
 Serial.print("Server is listening at: http://");
 Serial.print(WiFi.localIP());
 Serial.println("/");

 pinMode(PIN_LIGHT_SWITCH, OUTPUT);

 Serial.println("Device is ready.");
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
      return;
  }

  while(!client.available()){
      delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  if(request.indexOf(SWITCH_LIGHT_URL) != -1) {
    lightState = !lightState;
    if(lightState) {
      digitalWrite(PIN_LIGHT_SWITCH, HIGH);
    } else {
      digitalWrite(PIN_LIGHT_SWITCH, LOW);
    }
  }

  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>OK</html>\n";

  client.print(s);
  delay(1);
}
