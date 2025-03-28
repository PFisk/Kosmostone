#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

const char* ssid = "";
const char* password = "";
const char* colorKeyURL = "https://raw.githubusercontent.com/Kentswegge/Kosmotest/refs/heads/main/color%20keys.json";

// Store JSON data
DynamicJsonDocument colorKeyDoc(8192);  // Increased buffer size

void fetchColorKeys();

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  fetchColorKeys();
}

void loop() {
  // Do nothing - we only fetch once in setup()
}

void fetchColorKeys() {
  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL verification
  
  HTTPClient http;
  http.begin(client, colorKeyURL);

  Serial.println("Fetching Color Key JSON...");
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) { // Check if request was successful
    String payload = http.getString();
    DeserializationError error = deserializeJson(colorKeyDoc, payload);

    if (error) {
      Serial.print("❌ JSON Parsing Error: ");
      Serial.println(error.c_str());
      return;
    }

    Serial.println("✅ Color Key JSON Loaded!\n");

    for (JsonPair kv : colorKeyDoc.as<JsonObject>()) {  // Iterate through categories (colors)
        Serial.print("🔷 Color Category: ");
        Serial.println(kv.key().c_str());

        JsonArray variations = kv.value().as<JsonArray>();  // Get translations array

        for (JsonVariant entry : variations) {  // Iterate through translations
            String word = entry.as<String>();
            Serial.print("   - Native: ");
            Serial.println(word);

            // Extract text inside parentheses
            int startIdx = word.indexOf('(');
            int endIdx = word.indexOf(')');
            if (startIdx != -1 && endIdx != -1 && endIdx > startIdx) {
                String parenthesis = word.substring(startIdx + 1, endIdx);
                Serial.print("   - parenthesis: ");
                Serial.println(parenthesis);
            }
        }
        Serial.println(); // Add spacing between categories
    }
  } else {
    Serial.print("❌ Failed to fetch JSON, error code: ");
    Serial.println(httpCode);
  }

  http.end();
}
