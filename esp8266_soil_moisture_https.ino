/*
  ESP8266 + Soil Moisture Sensor -> HTTPS POST (JSON)

  Same behavior as the HTTP version, but uses WiFiClientSecure so it
  works with https:// endpoints (like the https link webhook.site gives you).

  IMPORTANT NOTE ON SECURITY:
  This uses client.setInsecure(), which skips SSL certificate validation.
  That's fine for testing/prototyping (webhook.site, quick demos), but is
  NOT recommended for a production system handling real student data --
  at that point, proper certificate validation should be set up instead.

  WIRING
    Soil sensor VCC -> 3.3V (or 5V if your module needs it, check its label)
    Soil sensor GND -> GND
    Soil sensor AO  -> A0

  LIBRARIES NEEDED
    - ArduinoJson (v6+)
    - ESP8266HTTPClient and WiFiClientSecure (both bundled with the
      ESP8266 board package, no separate install needed)
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ---------- WiFi config ----------
const char* WIFI_SSID     = "YOUR_WIFI";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ---------- Server endpoint ----------
// TODO: paste your https:// webhook.site URL (or your boss's real link) here
const char* SERVER_URL = "https://webhook.site/your-unique-id-here";

const int SOIL_PIN = A0;
const char* DEVICE_ID = "soil-sensor-1";

// ---------- Calibration ----------
// Replace these with real readings from your sensor: dry air vs fully wet
const int DRY_VALUE = 800;
const int WET_VALUE = 300;

const unsigned long SEND_INTERVAL_MS = 10000; // send every 10 seconds
unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  connectWiFi();
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL_MS) {
    lastSend = millis();
    readAndSend();
  }
}

void connectWiFi() {
  Serial.print(F("Connecting to WiFi"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print(F("WiFi connected, IP: "));
  Serial.println(WiFi.localIP());
}

String getMoistureStatus(int raw, int &percent) {
  percent = map(raw, DRY_VALUE, WET_VALUE, 0, 100);
  percent = constrain(percent, 0, 100);

  if (percent < 30) return "dry";
  else if (percent < 70) return "moist";
  else return "wet";
}

void readAndSend() {
  int raw = analogRead(SOIL_PIN);
  int percent;
  String status = getMoistureStatus(raw, percent);

  Serial.printf("Raw: %d  Percent: %d%%  Status: %s\n", raw, percent, status.c_str());

  StaticJsonDocument<200> doc;
  doc["device_id"] = DEVICE_ID;
  doc["raw_value"] = raw;
  doc["moisture_percent"] = percent;
  doc["status"] = status;

  String jsonPayload;
  serializeJson(doc, jsonPayload);

  sendHttpsPost(jsonPayload);
}

void sendHttpsPost(const String& payload) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("WiFi not connected, skipping send."));
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // skip certificate validation -- fine for testing only

  HTTPClient https;

  if (https.begin(client, SERVER_URL)) {
    https.addHeader("Content-Type", "application/json");

    int httpResponseCode = https.POST(payload);

    if (httpResponseCode > 0) {
      String response = https.getString();
      Serial.printf("HTTPS %d response: %s\n", httpResponseCode, response.c_str());
    } else {
      Serial.printf("HTTPS POST failed: %s\n", https.errorToString(httpResponseCode).c_str());
    }

    https.end();
  } else {
    Serial.println(F("Unable to connect to server (https.begin failed)."));
  }
}
