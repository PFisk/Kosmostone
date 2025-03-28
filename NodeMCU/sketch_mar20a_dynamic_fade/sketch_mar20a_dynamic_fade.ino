#include <FastLED.h>

// LED Configuration
#define LED_PIN     D5          // Pin where the LED data is connected
#define NUM_LEDS    3           // Number of LEDs in the strip
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

CRGB leds[NUM_LEDS];

// Simulated JSON Test Data
String testColorCases[] = {
  "green",
  "blue, red",
  "yellow, purple",
  "white, orange",
  "pink",
  "blue"
};

int testIndex = 0; // Tracks current test case

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  Serial.begin(9600);
  
  Serial.println("Starting LED Color Test...");
}

void loop() {
  // Read color data from simulated JSON
  String colorData = testColorCases[testIndex];

  Serial.print("Testing color: ");
  Serial.println(colorData);

  // Extract colors from the test case
  String colors[2] = {"", ""};  
  int commaIndex = colorData.indexOf(",");
  
  if (commaIndex != -1) {  
    colors[0] = colorData.substring(0, commaIndex);  
    colors[1] = colorData.substring(commaIndex + 2);  
  } else {
    colors[0] = colorData;
  }

  // Convert color names to actual RGB values
  CRGB primaryColor = getColorFromName(colors[0]);
  CRGB secondaryColor = (colors[1] != "") ? getColorFromName(colors[1]) : CRGB::Black;

  // Run the fade effect
  fadeEffect(primaryColor, secondaryColor);

  // Move to next test case
  testIndex = (testIndex + 1) % (sizeof(testColorCases) / sizeof(testColorCases[0]));
  
  delay(5000); // Wait before testing next color
}

// Function to convert color names to RGB values
CRGB getColorFromName(String color) {
  color.toLowerCase(); // Normalize case sensitivity

  if (color.indexOf("red") != -1) return CRGB::Red;
  if (color.indexOf("green") != -1) return CRGB(0, 255, 25); // Deep orange
  if (color.indexOf("blue") != -1) return CRGB::Blue;
  if (color.indexOf("yellow") != -1) return CRGB(255, 160, 0); // Deep orange
  if (color.indexOf("purple") != -1) return CRGB::Purple;
  if (color.indexOf("white") != -1) return CRGB::White;
  if (color.indexOf("orange") != -1) return CRGB(255, 40, 0); // Deep orange
  if (color.indexOf("pink") != -1) return CRGB(210, 60, 150); // Custom pink

  return CRGB::White; // Default unknown colors to white
}

// Fade effect function
void fadeEffect(CRGB primary, CRGB secondary) {
    unsigned long fadeDuration = 3000;
    unsigned long startTime = millis();

    // Fade in primary color
    while (millis() - startTime < fadeDuration) {
        float progress = float(millis() - startTime) / fadeDuration;
        int brightness = 255 * progress;

        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = primary;
        }

        FastLED.setBrightness(brightness);
        FastLED.show();
        delay(10);
    }

    // If there's a secondary color, blend smoothly into it
    if (secondary != CRGB::Black) {
        startTime = millis();
        while (millis() - startTime < fadeDuration) {
            float progress = float(millis() - startTime) / fadeDuration;
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = blend(primary, secondary, progress * 255); // Smooth blend
            }
            FastLED.show();
            delay(10);
        }
    }

    // Fade out smoothly
    startTime = millis();
    while (millis() - startTime < fadeDuration) {
        float progress = float(millis() - startTime) / fadeDuration;
        int brightness = 255 - (255 * progress);

        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = secondary != CRGB::Black ? secondary : primary; // Maintain last active color
        }

        FastLED.setBrightness(brightness);
        FastLED.show();
        delay(10);
    }

    delay(1000);
}
