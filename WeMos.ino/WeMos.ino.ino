//#define MEASURE_DHT

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#ifdef MEASURE_DHT
#include <DHT.h>
#endif

#ifdef MEASURE_DHT
#define DHTTYPE DHT22
#define PIN_DHT D7
#define DHT_TICKER_INTERVAL 5.0
#define DHT_INTERVAL 5000
DHT dht(PIN_DHT, DHTTYPE);
float humidity;
float temperature;
#endif

const char* ssid = "DIR-620";
const char* password = "76543210";

#define URL_SWITCH_RELAY  "/relay"
#define PIN_RELAY_0 D4
#define PIN_RELAY_1 D5
#define PIN_RELAY_2 D6
bool relayState[] = { false, false, false };
int relayPin[] = { PIN_RELAY_0, PIN_RELAY_1, PIN_RELAY_2 };

ESP8266WebServer server(80);

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
  while( WiFi.waitForConnectResult() != WL_CONNECTED ) {
    Serial.println("WiFi connection failed. Retry...");
    delay(10000);
    ESP.restart();
  }
  Serial.print("WiFi is ready. Local ip: ");
  Serial.println(WiFi.localIP());
}

void setupWiFiServer() {
  server.onNotFound([](){ server.send(404, "text/plain", "404 Not Found"); });
  server.on(URL_SWITCH_RELAY, [](){
    int relayID = server.arg("id").toInt();
    if( relayID < 0 || relayID > 2 ) {
      server.send(404, "text/plain", "404 Not Found"); 
    } else {
      relayState[relayID] = !relayState[relayID];
      digitalWrite(relayPin[relayID], (relayState[relayID] ? HIGH : LOW));
      server.send(200, "text/plain", "200 OK");
    }    
  });
  
  server.begin();
  Serial.print("Server is listening at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");  
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    continue;
  }
  Serial.println("Serial port is ready.");

  setupWiFi();
  setupWiFiServer();
  setupPins();

  #ifdef MEASURE_DHT
  dht.begin();
  Serial.println("DHT sensor is ready.");
  #endif

  Serial.println("Device is ready.");
}

#ifdef MEASURE_DHT
void measureDht() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "dht";
  root["status"] = (isnan(temperature) || isnan(humidity) ? "failed" : "success");
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root.prettyPrintTo(Serial);
}
#endif

void loop() {
  #ifdef MEASURE_DHT
  if( millis() % DHT_INTERVAL == 0 ) {
    measureDht();
  }
  #endif
  server.handleClient();
}
