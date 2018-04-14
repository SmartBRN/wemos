#define MEASURE_DHT
#define ENABLE_SERVER
//#define REGISTER_DEVICE

#include <ESP8266HTTPClient.h>
#define DEVICE_IP "192.168.0.100"
#define DEVICE_ID "0"
#define DEVICE_REGISTER_ROUTINE "0.0.0.0/register_device.php"

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>


#ifdef ENABLE_SERVER
#include <ESP8266WebServer.h>
#endif

#ifdef MEASURE_DHT
#include <DHT.h>
#endif

#ifdef MEASURE_DHT
#define DHTTYPE DHT22
#define PIN_DHT D7
#define DHT_TICKER_INTERVAL 5.0
#define DHT_INTERVAL 5000
#define URL_TEMPERATURE "/temperature"
#define URL_HUMIDITY "/humidity"
DHT dht(PIN_DHT, DHTTYPE);
float humidity;
float temperature;
#endif

const char* ssid = "DIR-620";
const char* password = "76543210";

const String availableSensors[] = { "temperature", "humidity" };
const String availableControls[] = { "relay", "relay" };
const int numberOfSensors = 2;
const int numberOfControls = 2;

#define URL_SWITCH_RELAY  "/relay"
#define PIN_RELAY_0 D5
#define PIN_RELAY_1 D6
bool relayState[] = { false, false };
int relayPin[] = { PIN_RELAY_0, PIN_RELAY_1 };

#ifdef ENABLE_SERVER
ESP8266WebServer server(80);
#endif

void setupPins() {
  pinMode(PIN_RELAY_0, OUTPUT);
  pinMode(PIN_RELAY_1, OUTPUT);
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

#ifdef ENABLE_SERVER
void setupServer() {
  server.onNotFound([](){ server.send(404, "text/plain", "404 Not Found"); });
  server.on(URL_SWITCH_RELAY, [](){
    int relayID = server.arg("id").toInt();
    if( relayID < 0 || relayID > 1 ) {
      server.send(404, "text/plain", "404 Not Found"); 
    } else {
      relayState[relayID] = !relayState[relayID];
      digitalWrite(relayPin[relayID], (relayState[relayID] ? HIGH : LOW));
      server.send(200, "text/plain", "200 OK");
    }    
  });
  #ifdef MEASURE_DHT
  server.on(URL_TEMPERATURE, [](){
    server.send(200, "text/plain", String(temperature) + " *C");
  });
  server.on(URL_HUMIDITY, [](){
    server.send(200, "text/plain", String(humidity) + " %");
  });
  #endif
  
  server.begin();
  Serial.print("Server is listening at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");  
}
#endif

#ifdef REGISTER_DEVICE
void registerDevice() {
  String json;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ip"] = WiFi.localIP().toString();
  root["id"] = DEVICE_ID;
  JsonArray& sensors = root.createNestedArray("sensors");
  for(int i = 0; i < numberOfSensors; ++i) {
    sensors.add(availableSensors[i]);
  }
  JsonArray& controls = root.createNestedArray("controls");
  for(int i = 0; i < numberOfControls; ++i) {
    controls.add(availableControls[i]);
  }
  root.printTo(json);  
  Serial.println(json);
  
  HTTPClient http;
  http.begin(DEVICE_REGISTER_ROUTINE);
  http.addHeader("Content-Type", "application/json");
  http.POST(json);
  http.end();  
}
#endif

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    continue;
  }
  Serial.println("Serial port is ready.");

  setupWiFi();
  
  #ifdef ENABLE_SERVER
  setupServer();
  #endif
  
  setupPins();

  #ifdef MEASURE_DHT
  dht.begin();
  Serial.println("DHT sensor is ready.");
  #endif

  #ifdef REGISTER_DEVICE
  registerDevice();
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
