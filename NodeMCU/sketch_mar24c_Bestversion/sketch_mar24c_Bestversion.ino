#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <WiFiClientSecure.h>
#include "config.h"  // File referencing wifi credentials

#define FASTLED_ESP8266_RAW_PIN_ORDER  
#define LED_PIN     D5          
#define NUM_LEDS    3           
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define BRIGHTNESS  255

// 🔹 Move this struct to the top
struct ExtractedColor {
    String native;
    String inParens;
};


// const char* ssid = "";
// const char* password = "";
const char* jsonURL = "https://raw.githubusercontent.com/Kentswegge/Kosmotest/refs/heads/main/fireballs.json";
const char* colorKeyURL = "https://raw.githubusercontent.com/Kentswegge/Kosmotest/refs/heads/main/color%20keys.json";

CRGB leds[NUM_LEDS];
DynamicJsonDocument colorKeyDoc(4096);

void fetchColorKeys();

void setup() {
  Serial.begin(115200);
  
  // ✅ Now this will work because the macros are properly defined
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  digitalWrite(LED_BUILTIN, HIGH);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  
  fetchColorKeys();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Fetching Fireballs JSON...");
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, jsonURL);

    int httpCode = http.GET();
    if (httpCode == 200) {
      String payload = http.getString();
      Serial.println("Received Fireballs JSON:");
      Serial.println(payload);

      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);
      JsonArray fireballs = doc["fireballs"].as<JsonArray>();

      for (JsonObject event : fireballs) {
        String rawColor = event["color"];
        String matchedColor = matchColor(rawColor);
        
        Serial.print("Extracted Color: ");
        Serial.print(rawColor);
        Serial.print(" (Matched: ");
        Serial.print(matchedColor);
        Serial.println(")");

        runColorSequence(matchedColor);
        delay(5000);
      }
    } else {
      Serial.print("HTTP Request failed, error code: ");
      Serial.println(httpCode);
    }
    http.end();
  }
  delay(3600000);
}

void fetchColorKeys() {
  Serial.println("Fetching Color Key JSON...");
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, colorKeyURL);

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Received Color Key JSON...");
    deserializeJson(colorKeyDoc, payload);
  } else {
    Serial.print("Failed to fetch color key JSON, error code: ");
    Serial.println(httpCode);
  }
  http.end();
}


ExtractedColor normalizeColor(String color) {
    color.toLowerCase();
    color.trim();
    color.replace("-", "");
    color.replace("  ", " ");

    int start = color.indexOf("(");
    int end = color.indexOf(")");

    ExtractedColor extracted;
    extracted.native = color;  // Default full name

    if (start != -1 && end != -1) {
        extracted.inParens = color.substring(start + 1, end);
        extracted.inParens.trim();
        extracted.native = color.substring(0, start); // **Fix: Assign first**
        extracted.native.trim();  // **Then trim separately**
    }

    Serial.print("🔹 Native: ");
    Serial.println(extracted.native);
    Serial.print("🔹 Inside Parentheses: ");
    Serial.println(extracted.inParens);

    return extracted;
}


String matchColor(String inputColor) {
    ExtractedColor extracted = normalizeColor(inputColor);
    Serial.print("🔍 Searching for Match: ");
    Serial.print(extracted.native);
    if (extracted.inParens.length() > 0) {
        Serial.print(" / ");
        Serial.print(extracted.inParens);
    }
    Serial.println();

    String bestMatch = "";
    int bestMatchLength = 0;

    for (JsonPair kv : colorKeyDoc.as<JsonObject>()) {
        String baseColor = kv.key().c_str();
        JsonArray variants = kv.value().as<JsonArray>();

        for (String variant : variants) {
            String normalizedVariant = normalizeColor(variant).native; // Extract native for comparison

            // **Perfect Match (Native)**
            if (extracted.native == normalizedVariant || extracted.inParens == normalizedVariant) {
                Serial.print("✅ Perfect Match Found: ");
                Serial.println(baseColor);
                return baseColor;
            }

            // **Substring Match - Longest Valid Match**
            if (extracted.native.indexOf(normalizedVariant) != -1 || extracted.inParens.indexOf(normalizedVariant) != -1) {
                if (normalizedVariant.length() > bestMatchLength) {
                    bestMatch = baseColor;
                    bestMatchLength = normalizedVariant.length();
                }
            }
        }
    }

    if (bestMatch != "") {
        Serial.print("🔸 Substring Match: ");
        Serial.println(bestMatch);
        return bestMatch;
    }

    Serial.println("❌ No match found, defaulting to white.");
    return "white";
}


void runColorSequence(String color) {
  CRGB targetColor = getColorFromName(color);
  
  for (int i = 0; i < 255; i++) { 
    FastLED.setBrightness(i);
    fill_solid(leds, NUM_LEDS, targetColor);
    FastLED.show();
    delay(10);
  }
  
  delay(1000);

  for (int i = 255; i > 0; i--) { 
    FastLED.setBrightness(i);
    FastLED.show();
    delay(10);
  }

  FastLED.setBrightness(0);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  Serial.println("LEDs fully turned off.");
}

CRGB getColorFromName(String color) {
  color.toLowerCase();
  
  if (color == "red") return CRGB::Red;
  if (color == "green") return CRGB(0, 255, 25);
  if (color == "blue") return CRGB::Blue;
  if (color == "yellow") return CRGB(255, 160, 0);
  if (color == "purple") return CRGB::Purple;
  if (color == "white") return CRGB::White;
  if (color == "orange") return CRGB(255, 40, 0);
  if (color == "pink") return CRGB(210, 60, 150);
  
  return CRGB::White;
}
