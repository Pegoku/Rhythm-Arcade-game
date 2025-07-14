#include <Arduino.h>
#include <FastLED.h>
#include <TM1637Display.h>

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

TM1637Display display(17, 5);
TM1637Display display2(13, 14);

// Variables for character control
char characters[4] = {'A', 'B', 'C', 'D'}; // Initial characters
int selectedIndex = 0; // Index of the currently selected character

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
  
  Serial.println("Setup complete");
}

void updateDisplay2() {
  // Update display2 with the current characters
  for (int i = 0; i < 4; i++) {
    display2.showNumberDecEx(characters[i], 0, true, 1, i);
    display2.
  }

  // Print the characters to the serial monitor
  Serial.print("Characters: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(characters[i]);
  }
  Serial.println();
}

void loop()
{
  /*
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
  delay(1000);
  */

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
