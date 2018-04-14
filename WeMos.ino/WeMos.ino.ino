#define MEASURE_DHT
#define MEASURE_GAS
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

#define MEASURE_INTERVAL 5000

#ifdef MEASURE_DHT
#define DHTTYPE DHT22
#define PIN_DHT D7
#define URL_TEMPERATURE "/temperature"
#define URL_HUMIDITY "/humidity"
#define URL_JSON_DHT "/json/dht"
DHT dht(PIN_DHT, DHTTYPE);
float humidity = 0.0;
float temperature = 0.0;
#endif

#ifdef MEASURE_GAS
#define URL_GAS_DETECTOR "/gas"
#define URL_JSON_GAS "/json/gas"
#define PIN_FC22 D2
bool gasDetected = false;
#endif

const char* ssid = "DIR-620";
const char* password = "76543210";

const String availableSensors[] = { "temperature", "humidity", "gas" };
const String availableControls[] = { "relay", "relay" };
const int numberOfSensors = 2;
const int numberOfControls = 2;

#define URL_SWITCH_RELAY  "/relay"
#define PIN_RELAY_0 D5
#define PIN_RELAY_1 D6
#define RELAY_0_ALIAS "relay 0"
#define RELAY_1_ALIAS "relay 1"
#define URL_JSON_RELAY "/json/relay"
const int numberOfRelays = 2;
bool relayState[] = { false, false };
int relayPin[] = { PIN_RELAY_0, PIN_RELAY_1 };
String relayAlias[] = { RELAY_0_ALIAS, RELAY_1_ALIAS };

#ifdef ENABLE_SERVER
ESP8266WebServer server(80);
#endif

void setupPins() {
  pinMode(PIN_RELAY_0, OUTPUT);
  pinMode(PIN_RELAY_1, OUTPUT);
  digitalWrite(PIN_RELAY_0, HIGH);
  digitalWrite(PIN_RELAY_1, HIGH);
  #ifdef MEASURE_GAS
  pinMode(PIN_FC22, INPUT);
  #endif
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
    String state = server.arg("state");
    if( relayID < 0 || relayID > 1 || !(state == "on" || state == "off") ) {
      server.send(404, "text/plain", "404 Not Found"); 
    } else {
      relayState[relayID] = (state == "on");
      digitalWrite(relayPin[relayID], (relayState[relayID] ? LOW : HIGH));
      server.send(200, "text/plain", "200 OK");
    }    
  });
  server.on(URL_JSON_RELAY, [](){
    int id = server.arg("id").toInt();
    server.send(200, "application/json", relayStateAsJson(id));
  });
  #ifdef MEASURE_DHT
  server.on(URL_TEMPERATURE, [](){
    server.send(200, "text/plain", String(temperature) + " *C");
  });
  server.on(URL_HUMIDITY, [](){
    server.send(200, "text/plain", String(humidity) + " %");
  });
  server.on(URL_JSON_DHT, [](){
    server.send(200, "application/json", dhtMeasurementAsJson());
  });
  #endif
  #ifdef MEASURE_GAS
  server.on(URL_GAS_DETECTOR, [](){
    server.send(200, "text/plain", (gasDetected ? "Gas detected: yes" : "Gas detected: no"));
  });
  server.on(URL_JSON_GAS, [](){
    server.send(200, "application/json", gasMeasurementAsJson());
  });
  #endif
  
  server.begin();
  Serial.print("Server is listening at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");  
}
#endif

String relayStateAsJson(int& id) {
  if( id < 0 || id >= numberOfRelays ) return "";
  String json;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject(); 
  root["id"] = id;
  root["alias"] = relayAlias[id];
  root["state"] = relayState[id] ? "on" : "off";
  root.printTo(json);
  return json;
}

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
}

String dhtMeasurementAsJson() {
  String json;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "dht";
  root["status"] = (isnan(temperature) || isnan(humidity) ? "failed" : "success");
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root.printTo(json);
  return json;
}
#endif

#ifdef MEASURE_GAS
void measureGas() {
  gasDetected = digitalRead(PIN_FC22) == HIGH;
}

String gasMeasurementAsJson() {
  String json;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["sensor"] = "gas";
  root["status"] = "success";
  root["gas"] = gasDetected;
  root.printTo(json);
  return json;
}
#endif

void loop() {  
  if( millis() % MEASURE_INTERVAL == 0 ) {
    #ifdef MEASURE_DHT
    measureDht();
    #endif

    #ifdef MEASURE_GAS
    measureGas();
    #endif
  }
  
  server.handleClient();
}
