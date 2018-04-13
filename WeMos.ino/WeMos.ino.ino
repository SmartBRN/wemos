#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>

#define DHTTYPE DHT22
#define PIN_DHT D3

#define HTTP_REPLY_OK "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>200 OK</html>\n"
#define HTTP_REPLY_BAD_REQUEST "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>400 Bad Request</html>\n"

#define URL_SWITCH_RELAY_0 "/relay/0"
#define URL_SWITCH_RELAY_1 "/relay/1"
#define URL_SWITCH_RELAY_2 "/relay/2"

const char* ssid = "DIR-620";
const char* password = "76543210";

#define PIN_RELAY_0 D5
#define PIN_RELAY_1 D6
#define PIN_RELAY_2 D7
bool relayState[] = { false, false, false };
int relayPins[] = { PIN_RELAY_0, PIN_RELAY_1, PIN_RELAY_2 };

WiFiServer server(80);

void setupPins() {
  pinMode(PIN_RELAY_0, OUTPUT);
  pinMode(PIN_RELAY_1, OUTPUT);
  pinMode(PIN_RELAY_2, OUTPUT);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);

  IPAddress ip(192, 168, 0, 100);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip, gateway, subnet);

  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi connection failed. Retry...");
    delay(10000);
    ESP.restart();
  }
  Serial.print("WiFi is ready. Local ip: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    continue;
  }
  Serial.println("Serial port is ready.");

  setupWiFi();


  server.begin();
  Serial.print("Server is listening at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  setupPins();

  Serial.println("Device is ready.");
}

void handleClientRequest() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  while (!client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  int relayID = -1;

  if ( request.indexOf(URL_SWITCH_RELAY_0) != -1 ) {
    relayID = 0;
  } else if ( request.indexOf(URL_SWITCH_RELAY_1) != -1 ) {
    relayID = 1;
  } else if ( request.indexOf(URL_SWITCH_RELAY_2) != -1 ) {
    relayID = 2;
  }

  if ( relayID == -1 ) {
    client.print(HTTP_REPLY_BAD_REQUEST);
  } else {
    relayState[relayID] = !relayState[relayID];
    digitalWrite(relayPins[relayID], (relayState[relayID] ? HIGH : LOW));
    client.print(HTTP_REPLY_OK);
  }

  delay(1);
}

void loop() {
  handleClientRequest();
}
