#include <ArduinoJson.h>
#include <FastLED.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "config.h"

#define LED_PIN    D2          // Change to the pin you're using for the LED strip
#define NUM_LEDS   30          // Number of LEDs in the strip
#define CHIPSET    WS2812      // Type of LED strip (can be WS2812, SK6812, etc.)
#define COLOR_ORDER GRB        // Color order for the LEDs
CRGB leds[NUM_LEDS];          // Create an array to store LED colors

// URL to the JSON file containing color keys
const char* colorKeyURL = "https://raw.githubusercontent.com/Kentswegge/Kosmotest/refs/heads/main/color%20keys.json";

// Function to fetch the color keys from the JSON file
DynamicJsonDocument fetchColorKeys() {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, colorKeyURL);
  
  int httpCode = http.GET();
  DynamicJsonDocument doc(2048);

  if (httpCode == 200) {
    String payload = http.getString();
    deserializeJson(doc, payload);
  } else {
    Serial.println("Failed to fetch color keys JSON");
  }

  http.end();
  return doc;
}

// Function to normalize the input color string
String normalizeInput(String input) {
  input.toLowerCase();
  input.trim(); // Remove spaces
  return input;
}

// Function to match an input color to the color keys
String matchColor(String input, DynamicJsonDocument colorKeys) {
  String normalizedInput = normalizeInput(input);
  
  for (JsonPair kv : colorKeys.as<JsonObject>()) {
    String baseColor = kv.key().c_str();
    JsonArray variants = kv.value().as<JsonArray>();

    // Loop through each variant (translation) for the base color
    for (String variant : variants) {
      String normalizedVariant = normalizeInput(variant);

      // If there's a match, return the base color
      if (normalizedInput == normalizedVariant) {
        return baseColor;
      }
    }
  }

  // If no match is found, return "white"
  return "white";
}

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Wait for a bit
    Serial.print(".");
  }
  
  Serial.println("\nConnected to WiFi!");
  digitalWrite(LED_BUILTIN, HIGH);

  delay(1000); // Wait before proceeding

  // Fetch the color keys JSON
  DynamicJsonDocument colorKeys = fetchColorKeys();

  delay(1000); // Wait before printing the results

  // Test inputs
  String testInputs[] = {
    "Lys gul", "White", "Green", "青", "bun-hong-saek", 
    "Weiß", "appelsínugult", "paars", "vihreä", "красный"
  };

  // Match each input and print the result
  for (String input : testInputs) {
    String matchedColor = matchColor(input, colorKeys);
    Serial.print("Input: ");
    Serial.print(input);
    Serial.print(" → Matched Color: ");
    Serial.println(matchedColor);
  }
}

// Loop function is mandatory in an Arduino sketch
void loop() {
  // Empty loop for now
  delay(3000);  // Just a delay to keep the loop running
}
