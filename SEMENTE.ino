#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DHTPIN 2
#define DHTTYPE DHT11
const int dry = 595;
const int wet = 239;

DHT dht(DHTPIN, DHTTYPE);

short int httpResponseCode;
const char* ssid = "sitio117";
const char* password =  "sitio117";
const char* serverUrl = "http://localhost:3334/api/measurement/";

String httpRequestData,payload;

bool waterPumpStatus = false;
int plantId = 1;

void networkConnect(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("IP: " + (String)WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  networkConnect();
}

String formatPlantDataToJson(int plantId, float soilTemperature, float soilHumidity, float airTemperature, float airHumidity, bool waterPumpStatus) {
  const size_t capacity = JSON_OBJECT_SIZE(6);
  DynamicJsonDocument jsonDoc(capacity);

  jsonDoc["plantId"] = plantId;
  jsonDoc["soilTemperature"] = soilTemperature;
  jsonDoc["soilHumidity"] = soilHumidity;
  jsonDoc["airTemperature"] = airTemperature;
  jsonDoc["airHumidity"] = airHumidity;
  jsonDoc["waterPumpStatus"] = waterPumpStatus;

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  return jsonString;
}

void sendMeasurementData(int plantId, float soilTemperature, float soilHumidity, float airTemperature, float airHumidity, bool waterPumpStatus) {
  HTTPClient http;

  const size_t capacity = JSON_OBJECT_SIZE(6);
  DynamicJsonDocument jsonDoc(capacity);

  jsonDoc["plantId"] = plantId;
  jsonDoc["soilTemperature"] = soilTemperature;
  jsonDoc["soilHumidity"] = soilHumidity;
  jsonDoc["airTemperature"] = airTemperature;
  jsonDoc["airHumidity"] = airHumidity;
  jsonDoc["waterPumpStatus"] = waterPumpStatus;

  String jsonString;
  serializeJson(jsonDoc, jsonString);

  http.begin(serverUrl);

  http.addHeader("Content-Type", "application/json; charset=utf-8");

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.println("Resposta do servidor: ");
    Serial.println(http.getString());
  } else {
    Serial.println("Request error. Code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

float readAirTemperature(){
  float airTemperature = dht.readTemperature();
  if(!isnan(airTemperature)){
    return airTemperature;
  } else {
    return 0.0f;
  }
}

float readAirHumidity(){
  float airHumidity = dht.readHumidity();
  if(!isnan(airHumidity)){
    return airHumidity;
  } else {
    return 0.0f;
  }
}

int soilMoisture(){
  int soilMoisture = map(analogRead(23), wet, dry, 100, 0);

  return soilMoisture;
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    delay(5000);
    // Serial.println("Data Sent:");
    // Serial.println("Temperatura do Ar: ");
    // Serial.println(readAirTemperature());
    // Serial.println("Umidade do Ar: ");
    // Serial.println(readAirHumidity());
    // Serial.println("Umidade do Solo: ");
    // Serial.println(soilMoisture());
    // Serial.println("Status da bomba: ");
    // Serial.println(waterPumpStatus);

    String jsonData = formatPlantDataToJson(plantId, 0, soilMoisture(), readAirTemperature(), readAirHumidity(), waterPumpStatus);
    sendMeasurementData(plantId, 0, soilMoisture(), readAirTemperature(), readAirHumidity(), waterPumpStatus);
  } else {
    Serial.println("WiFi não conectado. Aguardando...");
    networkConnect();
  }
}
