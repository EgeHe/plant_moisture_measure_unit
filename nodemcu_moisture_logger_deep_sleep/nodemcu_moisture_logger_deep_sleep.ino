#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";
const char* host = "";

const char* station = "";
const int analogPin = A0; // Naming analog input pin
const int maxPin = D6;
const int minPin = D5;

int inputVal = 0;        // Variable to store analog input values
String stringData, postData;

HTTPClient http;

bool connect() {
  Serial.println();
  WiFi.begin(ssid, password);
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF); // Prevents reconnection issue (taking too long to connect)
  WiFi.mode(WIFI_STA); // This line hides the viewing of ESP as wifi hotspot
  Serial.flush();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  unsigned long wifiConnectStart = millis();
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(3000);
    }
    delay(500);
    Serial.print(".");
    if (millis() - wifiConnectStart > 15000) {
      Serial.println("Failed to connect to WiFi");
      return false;
    }
  }
  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void postDataToServer(String data) {
  StaticJsonDocument<200> doc;
  doc["station"] = station;

  int is_min = digitalRead(minPin);
  int is_max = digitalRead(maxPin);

  if (digitalRead(minPin) == LOW) {
    Serial.println("minpin lowered");
    doc["min_moist"] = stringData;
  } else if (digitalRead(maxPin) == LOW) {
    Serial.println("maxpin lowered");
    doc["max_moist"] = stringData;
  } else {
    Serial.println("Sending moisture data");
    doc["moisture"] = stringData;
  }
  
  serializeJson(doc, postData);
  // postData = "{\"moisture\":" + stringData + ", \"station\": \"" + station + "\"}";
  Serial.print("Posting data: ");
  Serial.println(postData);
  http.begin(host);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(postData);
  String payload = http.getString();    //Get the response payload
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
  http.end();  //Close connection
}
 
void setup() {
  unsigned long sequenceStart = millis();
  pinMode(minPin, INPUT);
  pinMode(maxPin, INPUT);
  Serial.begin(115200);
  Serial.setTimeout(2000);
  // Wait for serial to initialize.
  while (!Serial) { }
  
  if (connect()) {
    inputVal = analogRead(analogPin); // Analog Values 0 to 1023
    stringData = String(inputVal);
    postDataToServer(stringData);
  }
  unsigned long msToSleep = 10000 - (millis() - sequenceStart);
  unsigned long usToSleep = msToSleep * 1000;
  Serial.print("going to deep sleep ");
  Serial.println(String(usToSleep / 1000000));
  ESP.deepSleep(usToSleep); // 10 seconds of deep sleep
}

void loop() {
}
