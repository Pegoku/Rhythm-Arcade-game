#include <Arduino.h>
#include <FastLED.h>
#include <TM1637Display.h>
#include <map>

#define LED1_PIN 2
#define LED2_PIN 23
#define LED3_PIN 18
#define LED4_PIN 19

#define BUTTON1_PIN 0
#define BUTTON2_PIN 4
#define BUTTON3_PIN 16
#define BUTTON4_PIN 21
#define BUTTON_UP_PIN 26
#define BUTTON_DOWN_PIN 25
#define BUTTON_RIGHT_PIN 32
#define BUTTON_LEFT_PIN 33

#define NUM_LEDS 5
#define BRIGHTNESS 20
#define LED_TYPE WS2812B

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

int led1_pos = 0;
int led2_pos = 0;
int led3_pos = 0;
int led4_pos = 0;

int points1 = 0;
int bestScore = 0;
String nameBestScore = "AAAA";

TM1637Display display(17, 5);
TM1637Display display2(13, 14);
TM1637Display display3(22, 15); // Additional display for character control


// Variables for character control
char characters[4] = {'A', 'B', 'C', 'D'}; // Initial characters
int selectedIndex = 0; // Index of the currently selected character

// 7-segment encodings for common letters (A-Z, limited by display capability)
std::map<char, uint8_t> letterEncoding = {
  {'A', 0b01110111},
  {'B', 0b01111100}, // b
  {'C', 0b00111001},
  {'D', 0b01011110}, // d
  {'E', 0b01111001},
  {'F', 0b01110001},
  {'G', 0b00111101},
  {'H', 0b01110110},
  {'I', 0b00110000},
  {'J', 0b00011110},
  {'K', 0b01110110}, // Same as H
  {'L', 0b00111000},
  {'M', 0b00110111}, // Approximation
  {'N', 0b01010100},
  {'O', 0b00111111},
  {'P', 0b01110011},
  {'Q', 0b01100111},
  {'R', 0b01010000},
  {'S', 0b01101101},
  {'T', 0b01111000},
  {'U', 0b00111110},
  {'V', 0b00111110}, // Same as U
  {'W', 0b00111110}, // Same as U/V
  {'X', 0b01110110}, // Same as H/K
  {'Y', 0b01101110},
  {'Z', 0b01011011}
  // Add more as needed, some letters can't be represented well
};

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED1_PIN, GRB>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED2_PIN, GRB>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED3_PIN, GRB>(leds3, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED4_PIN, GRB>(leds4, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // seed the RNG for variable speed and strip choice
  randomSeed(micros());

  display.setBrightness(0x0f);
  display.clear();
  display.showNumberDec(points1);
  
  display2.setBrightness(0x0f);
  display2.clear();
  display2.showNumberDec(bestScore); // Initialize display2 with bestScore

  display3.setBrightness(0x0f);
  display3.clear();
  display3.setSegments(8888); // Initialize display3 with 8888

  Serial.println("Setup complete");
}

void updateDisplay2() {
  // Map characters to 7-segment display segments using letterEncoding
  uint8_t segments[4];
  for (int i = 0; i < 4; i++) {
    if (letterEncoding.find(characters[i]) != letterEncoding.end()) {
      segments[i] = letterEncoding[characters[i]]; // Get the segment encoding from the map
    } else {
      segments[i] = 0; // Default to blank if character is not in the map
    }
  }

  // Update display2 with the segments
  display2.setSegments(segments);

  // Print the characters to the serial monitor
  Serial.print("Characters: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(characters[i]);
  }
  Serial.println();
}

void loop()
{

  // random leds and button
  int stripIndex = random(0, 4);
  CRGB* currentLeds;
  uint8_t buttonPin;
  CRGB activeColor;

  switch (stripIndex) {
    case 0:
      currentLeds = leds1;
      buttonPin = BUTTON1_PIN;
      activeColor = CRGB::Red;
      break;
    case 1:
      currentLeds = leds2;
      buttonPin = BUTTON2_PIN;
      activeColor = CRGB::Green;
      break;
    case 2:
      currentLeds = leds3;
      buttonPin = BUTTON3_PIN;
      activeColor = CRGB::Blue;
      break;
    case 3:
      currentLeds = leds4;
      buttonPin = BUTTON4_PIN;
      activeColor = CRGB::Yellow;
      break;
  }

  // random delay between steps (150ms–500ms)
  unsigned long timeout = random(150, 501);

  int buttonPressed = 0;
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // scan from last to first
  for (int idx = NUM_LEDS - 1; idx >= 0; idx--)
  {
    fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
    currentLeds[idx] = activeColor;
    FastLED.show();

    unsigned long startTime = millis();
    bool scored = false;

    // button press detection
    while (millis() - startTime < timeout)
    {
      if (digitalRead(buttonPin) == LOW)
      {
        buttonPressed++;
        if (idx == 0)
          points1 += 2;
        else if (idx == 1)
          points1 += 1;
        else
          points1 -= 2;
        scored = true;
        break;
      }
      delay(10);
    }

    if (scored) 
      break;
  }

  if (buttonPressed == 0)
    points1 -= 2;

  // display results
  display.showNumberDec(points1);
  Serial.println(points1);
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(0);
  
  // END GAME (after )

  // Button handling for display2
  if (digitalRead(BUTTON_UP_PIN) == LOW) { // Up button
    characters[selectedIndex]++;
    if (characters[selectedIndex] > 'Z') characters[selectedIndex] = 'A'; // Wrap around
    updateDisplay2();
    delay(200); // Debounce delay
  }

  if (digitalRead(BUTTON_DOWN_PIN) == LOW) { // Down button
    characters[selectedIndex]--;
    if (characters[selectedIndex] < 'A') characters[selectedIndex] = 'Z'; // Wrap around
    updateDisplay2();
    delay(200); // Debounce delay
  }

  if (digitalRead(BUTTON_RIGHT_PIN) == LOW) { // Right button
    selectedIndex = (selectedIndex + 1) % 4; // Move to the next character
    updateDisplay2();
    delay(200); // Debounce delay
  }

  if (digitalRead(BUTTON_LEFT_PIN) == LOW) { // Left button
    selectedIndex = (selectedIndex - 1 + 4) % 4; // Move to the previous character
    updateDisplay2();
    delay(200); // Debounce delay
  }
}
