#include <Arduino.h>
#include <FastLED.h>
#include <TM1637Display.h>
#include <map>

#define LED1_PIN 19
#define LED2_PIN 18
#define LED3_PIN 23
#define LED4_PIN 2

#define BUTTON1_PIN 25
#define BUTTON2_PIN 26
#define BUTTON3_PIN 33
#define BUTTON4_PIN 32
#define BUTTON_UP_PIN 4
#define BUTTON_DOWN_PIN 0
#define BUTTON_RIGHT_PIN 21
#define BUTTON_LEFT_PIN 16

#define NUM_LEDS 5
#define BRIGHTNESS 20
#define LED_TYPE WS2812B
#define GAME_DURATION (10 * 1000) // Game duration in ms (10 seconds)

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

int led1_pos = 0;
int led2_pos = 0;
int led3_pos = 0;
int led4_pos = 0;

int points1 = 0;
int bestScore = -40;
char nameBestScore[5] = "NAME";

TM1637Display display(17, 5);
TM1637Display display2(13, 14);
TM1637Display display3(22, 15); 


char characters[4] = {'A', 'B', 'C', 'D'}; // chars
int selectedIndex = 0; 

// 7-segment encodings for letters (A-Z)
std::map<char, uint8_t> letterEncoding = {
  {'A', 0b01110111},
  {'B', 0b01111100},
  {'C', 0b00111001},
  {'D', 0b01011110},
  {'E', 0b01111001},
  {'F', 0b01110001},
  {'G', 0b00111101},
  {'H', 0b01110110},
  {'I', 0b00110000},
  {'J', 0b00011110},
  {'K', 0b01110110},
  {'L', 0b00111000},
  {'M', 0b00110111}, // kinda
  {'N', 0b01010100},
  {'O', 0b00111111},
  {'P', 0b01110011},
  {'Q', 0b01100111},
  {'R', 0b01010000},
  {'S', 0b01101101},
  {'T', 0b01111000},
  {'U', 0b00111110},
  {'V', 0b00111110}, // Same as U/W
  {'W', 0b00111110}, // Same as U/V
  {'X', 0b01110110}, // Same as H/K
  {'Y', 0b01101110},
  {'Z', 0b01011011}
};

unsigned long gameStartTime = 0; 
bool gameActive = true;          

// track last state of row buttons
bool lastRowButtonState[4] = { HIGH, HIGH, HIGH, HIGH };

void updateDisplay2() {
  // using letterEncoding map characters to 7-segment display segments 
  uint8_t segments[4];
  for (int i = 0; i < 4; i++) {
    if (letterEncoding.find(characters[i]) != letterEncoding.end()) {
      segments[i] = letterEncoding[characters[i]]; // Get the segment encoding from map
    } else {
      segments[i] = 0; // Default to blank if character is not inmap
    }
  }

  display2.setSegments(segments);

  // Print characters to the serial monitor
  Serial.print("Characters: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(characters[i]);
  }
  Serial.println();
}

void displayTextOnTM1637(const char* text, TM1637Display& disp) {
  uint8_t segments[4] = {0, 0, 0, 0};
  for (int i = 0; i < 4 && text[i] != '\0'; i++) {
    if (letterEncoding.find(text[i]) != letterEncoding.end()) {
      segments[i] = letterEncoding[text[i]];
    } else {
      segments[i] = 0;
    }
  }
  disp.setSegments(segments);

  // Print characters to the serial monitor
  Serial.print("Display: ");
  for (int i = 0; i < 4 && text[i] != '\0'; i++) {
    Serial.print(text[i]);
  }
  Serial.println();
}

void waitForRestart() {
  // display.showNumberDecEx(0, 0b01111111, true, 4, 0); // "----" placeholder
  Serial.println("Press any button to start...");

  while (true) {
    if (digitalRead(BUTTON1_PIN) == LOW || digitalRead(BUTTON2_PIN) == LOW || 
        digitalRead(BUTTON3_PIN) == LOW || digitalRead(BUTTON4_PIN) == LOW || 
        digitalRead(BUTTON_UP_PIN) == LOW || digitalRead(BUTTON_DOWN_PIN) == LOW || 
        digitalRead(BUTTON_RIGHT_PIN) == LOW || digitalRead(BUTTON_LEFT_PIN) == LOW) {
      delay(200); // Debounce delay
      break;
    }
  }

  // Reset game
  points1 = 0;
  gameStartTime = millis();
  gameActive = true;
  display.showNumberDec(points1); // Reset the main display to show the score
  display2.showNumberDec(bestScore); // Keep showing the high score
  Serial.println("Game restarted!");
}

void blinkSelectedCharacter() {
  static unsigned long lastBlink = 0;
  static bool showChar = true;
  
  if (millis() - lastBlink > 300) {
    showChar = !showChar;
    lastBlink = millis();
    
    uint8_t segments[4];
    for (int i = 0; i < 4; i++) {
      if (i == selectedIndex && !showChar) {
        segments[i] = 0; // Blank the selected character to make it blink
      } else if (letterEncoding.find(characters[i]) != letterEncoding.end()) {
        segments[i] = letterEncoding[characters[i]];
      } else {
        segments[i] = 0;
      }
    }
    display.setSegments(segments);
  }
}

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
  
  display2.setBrightness(0x0f);
  display2.clear();


  display3.setBrightness(0x0f);
  display3.clear();


  gameStartTime = millis(); // Initialize the game start time

  Serial.println("Setup complete");

  displayTextOnTM1637(nameBestScore, display3); // Show the final score
  display2.showNumberDec(bestScore); // Keep showing the high score
  displayTextOnTM1637("PLAY", display); // Show the final score
  waitForRestart(); // Wait for any button press to restart
}



void loop()
{
  if (gameActive)
  {
    // Check if the game duration has passed
    if (millis() - gameStartTime >= GAME_DURATION)
    {
      gameActive = false; // End the game
      Serial.println("Time is Over!");

      // Check if the final score exceeds the high score
      if (points1 > bestScore)
      {
        Serial.println("New High Score!");
        displayTextOnTM1637("NAME", display); // Show characters on main display
        display2.showNumberDec(points1); 
        delay(1000);
      }
      else
      {
        Serial.println("No new high score.");
        displayTextOnTM1637("Over", display);
        display2.showNumberDec(bestScore);
        displayTextOnTM1637("PLAY", display);
        waitForRestart();
      }
    }
    else
    {
      // reset any released buttons so next press is detected
      for (int i = 0; i < 4; i++) {
        uint8_t pin = (i==0 ? BUTTON1_PIN :
                       i==1 ? BUTTON2_PIN :
                       i==2 ? BUTTON3_PIN :
                              BUTTON4_PIN);
        if (digitalRead(pin) == HIGH) lastRowButtonState[i] = HIGH;
      }

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
          bool curr = (digitalRead(buttonPin) == LOW);
          if (curr && lastRowButtonState[stripIndex]) {
            lastRowButtonState[stripIndex] = LOW;   // mark as handled
            buttonPressed++;
            if (idx == 0)        points1 += 2;
            else if (idx == 1)   points1 += 1;
            else                  points1 -= 2;
            scored = true;
            break;
          }
          lastRowButtonState[stripIndex] = curr ? LOW : HIGH;
          delay(10);
        }

        if (scored) break;
      }

      if (buttonPressed == 0) points1 -= 2;

      // display results
      display.showNumberDec(points1);
      Serial.println(points1);
      fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(0);
    }
  }
  else
  {
    // Username selection logic (only active after the game ends and if there's a new high score)
    if (points1 > bestScore)
    {
      blinkSelectedCharacter(); // Blink selected character

      if (digitalRead(BUTTON_UP_PIN) == LOW)
      {
        characters[selectedIndex]++;
        if (characters[selectedIndex] > 'Z') characters[selectedIndex] = 'A';
        delay(200);
      }

      if (digitalRead(BUTTON_DOWN_PIN) == LOW)
      {
        characters[selectedIndex]--;
        if (characters[selectedIndex] < 'A') characters[selectedIndex] = 'Z';
        delay(200);
      }

      if (digitalRead(BUTTON_RIGHT_PIN) == LOW)
      {
        selectedIndex = (selectedIndex + 1) % 4;
        delay(200);
      }

      if (digitalRead(BUTTON_LEFT_PIN) == LOW)
      {
        selectedIndex = (selectedIndex - 1 + 4) % 4;
        delay(200);
      }

      if (digitalRead(BUTTON1_PIN) == LOW || digitalRead(BUTTON2_PIN) == LOW ||
          digitalRead(BUTTON3_PIN) == LOW || digitalRead(BUTTON4_PIN) == LOW) {
        // Save the selected name and update high score
        bestScore = points1;
        strncpy(nameBestScore, characters, 4);
        nameBestScore[4] = '\0';
        Serial.print("Name saved: ");
        Serial.println(nameBestScore);
        
        // Update display3 with the new username
        displayTextOnTM1637(nameBestScore, display3);
        display2.showNumberDec(bestScore); // Update display2 with the new high score

        displayTextOnTM1637("PLAY", display);
        delay(2000);
        waitForRestart();
      }
    }
  }
  delay(10);
}