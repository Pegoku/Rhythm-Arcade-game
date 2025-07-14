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

TM1637Display display(17, 5);

// Game mode toggle
bool useRandomMode = false; // Set to true for random mode, false for song mode

// Song structure
struct Note {
  int stripIndex;  // 0-3 for strips, -1 for blank/pause
  unsigned long startTime; // When this note starts (milliseconds from song start)
  unsigned long duration; // How long this LED stays on
};

// Define songs
Note song1[] = {
  {0, 400},   // Red strip, 400ms
  {1, 300},   // Green strip, 300ms
  {-1, 200},  // Blank/pause, 200ms
  {2, 500},   // Blue strip, 500ms
  {3, 350},   // Yellow strip, 350ms
  {0, 250},   // Red strip, 250ms
  {1, 400},   // Green strip, 400ms
  {-1, 300},  // Blank/pause, 300ms
};

Note song2[] = {
  {1, 200},   // Fast green
  {1, 200},   // Fast green again
  {0, 600},   // Slow red
  {3, 300},   // Medium yellow
  {2, 400},   // Medium blue
  {-1, 500},  // Long pause
};

// Song management
int currentSong = 0; // 0 for song1, 1 for song2
int currentNoteIndex = 0;
Note* currentSongArray;
int currentSongLength;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED1_PIN, GRB>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED2_PIN, GRB>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED3_PIN, GRB>(leds3, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED4_PIN, GRB>(leds4, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // seed the RNG for variable speed and strip choice
  randomSeed(micros());

  // Initialize song
  if (currentSong == 0) {
    currentSongArray = song1;
    currentSongLength = sizeof(song1) / sizeof(Note);
  } else {
    currentSongArray = song2;
    currentSongLength = sizeof(song2) / sizeof(Note);
  }

  display.setBrightness(0x0f);
  display.clear();
  display.showNumberDec(points1);
  
  Serial.println("Setup complete");
}

void playRandomMode() {
  // Your existing random code
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

  unsigned long timeout = random(150, 501);

  int buttonPressed = 0;
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  for (int idx = NUM_LEDS - 1; idx >= 0; idx--) {
    fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
    currentLeds[idx] = activeColor;
    FastLED.show();

    unsigned long startTime = millis();
    bool scored = false;

    while (millis() - startTime < timeout) {
      if (digitalRead(buttonPin) == LOW) {
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

  display.showNumberDec(points1);
  Serial.println(points1);
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(1000);
}

void playSongMode() {
  // Get current note
  Note currentNote = currentSongArray[currentNoteIndex];
  
  // If it's a blank/pause note
  if (currentNote.stripIndex == -1) {
    // Turn off all LEDs and wait
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    fill_solid(leds2, NUM_LEDS, CRGB::Black);
    fill_solid(leds3, NUM_LEDS, CRGB::Black);
    fill_solid(leds4, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(currentNote.duration);
    
    // Move to next note
    currentNoteIndex++;
    if (currentNoteIndex >= currentSongLength) {
      currentNoteIndex = 0; // Loop the song
    }
    return;
  }

  // Normal note with strip
  CRGB* currentLeds;
  uint8_t buttonPin;
  CRGB activeColor;

  switch (currentNote.stripIndex) {
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

  int buttonPressed = 0;
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  for (int idx = NUM_LEDS - 1; idx >= 0; idx--) {
    fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
    currentLeds[idx] = activeColor;
    FastLED.show();

    unsigned long startTime = millis();
    bool scored = false;

    while (millis() - startTime < currentNote.duration) {
      if (digitalRead(buttonPin) == LOW) {
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

  display.showNumberDec(points1);
  Serial.println(points1);
  fill_solid(currentLeds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  // Move to next note
  currentNoteIndex++;
  if (currentNoteIndex >= currentSongLength) {
    currentNoteIndex = 0; // Loop the song
  }
  
  delay(100); // Small delay between notes
}

void loop()
{
  if (useRandomMode) {
    playRandomMode();
  } else {
    playSongMode();
  }
}