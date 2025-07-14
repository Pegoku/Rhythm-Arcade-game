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

// Define songs (fixed timing - using proper milliseconds)
Note song1[] = {
  {0, 0, 1000},     // Red starts at 0ms, lasts 1000ms
  {1, 500, 800},    // Green starts at 500ms, lasts 800ms
  {2, 1200, 1000},  // Blue starts at 1200ms, lasts 1000ms
  {3, 1800, 800},   // Yellow starts at 1800ms, lasts 800ms
  {0, 2500, 600},   // Red again at 2500ms
  {1, 3000, 800},   // Green at 3000ms
  {2, 3600, 700},   // Blue at 3600ms
  {-1, 4500, 1000}, // Pause/end marker at 4500ms
};

Note song2[] = {
  {1, 0, 600},      // Fast green
  {0, 200, 800},    // Red overlaps with green
  {3, 400, 600},    // Yellow starts while red is still on
  {2, 700, 800},    // Blue starts while yellow and red are on
  {0, 1200, 500},   // Quick red
  {1, 1500, 500},   // Quick green
  {-1, 2200, 500},  // End marker
};

// Song management
int currentSong = 0; // 0 for song1, 1 for song2
int currentNoteIndex = 0;
Note* currentSongArray;
int currentSongLength;
unsigned long songStartTime = 0;
bool songActive = false;

// Button press tracking for song mode
bool buttonPressed[4] = {false, false, false, false};
unsigned long lastButtonPress[4] = {0, 0, 0, 0};
const unsigned long debounceDelay = 50;

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

void playSongMode() {
  if (!songActive) {
    songStartTime = millis();
    songActive = true;
    currentNoteIndex = 0;
    // Reset button press tracking
    for(int i = 0; i < 4; i++) {
      buttonPressed[i] = false;
      lastButtonPress[i] = 0;
    }
  }

  unsigned long currentTime = millis() - songStartTime;
  
  // Clear all LEDs first
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  fill_solid(leds3, NUM_LEDS, CRGB::Black);
  fill_solid(leds4, NUM_LEDS, CRGB::Black);

  bool anyActive = false;
  
  // Check all notes to see which should be active now
  for (int i = 0; i < currentSongLength; i++) {
    Note note = currentSongArray[i];
    
    // Skip pause notes
    if (note.stripIndex == -1) continue;
    
    // Check if this note should be active
    if (currentTime >= note.startTime && currentTime < (note.startTime + note.duration)) {
      anyActive = true;
      
      CRGB* currentLeds;
      uint8_t buttonPin;
      CRGB activeColor;
      int buttonIndex;
      
      switch (note.stripIndex) {
        case 0:
          currentLeds = leds1;
          buttonPin = BUTTON1_PIN;
          activeColor = CRGB::Red;
          buttonIndex = 0;
          break;
        case 1:
          currentLeds = leds2;
          buttonPin = BUTTON2_PIN;
          activeColor = CRGB::Green;
          buttonIndex = 1;
          break;
        case 2:
          currentLeds = leds3;
          buttonPin = BUTTON3_PIN;
          activeColor = CRGB::Blue;
          buttonIndex = 2;
          break;
        case 3:
          currentLeds = leds4;
          buttonPin = BUTTON4_PIN;
          activeColor = CRGB::Yellow;
          buttonIndex = 3;
          break;
      }
      
      // Calculate LED position based on time within this note
      unsigned long noteProgress = currentTime - note.startTime;
      float progress = (float)noteProgress / note.duration;
      int ledIndex = NUM_LEDS - 1 - (int)(progress * NUM_LEDS);
      
      if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
        currentLeds[ledIndex] = activeColor;
        
        // Check for button press with debouncing
        if (digitalRead(buttonPin) == LOW && !buttonPressed[buttonIndex]) {
          if (millis() - lastButtonPress[buttonIndex] > debounceDelay) {
            buttonPressed[buttonIndex] = true;
            lastButtonPress[buttonIndex] = millis();
            
            // Score based on LED position
            if (ledIndex == 0) {
              points1 += 10; // Perfect hit
            } else if (ledIndex == 1) {
              points1 += 5;  // Good hit
            } else if (ledIndex == 2) {
              points1 += 2;  // Okay hit
            } else {
              points1 -= 1;  // Poor timing
            }
            
            display.showNumberDec(points1);
            Serial.print("Score: ");
            Serial.println(points1);
          }
        }
        
        // Reset button press flag when button is released
        if (digitalRead(buttonPin) == HIGH) {
          buttonPressed[buttonIndex] = false;
        }
      }
    }
  }
  
  FastLED.show();
  
  // Check if song is finished
  if (!anyActive && currentTime > (currentSongArray[currentSongLength-1].startTime + currentSongArray[currentSongLength-1].duration)) {
    songActive = false; // Restart song
    Serial.println("Song finished, restarting...");
  }
  
  delay(10); // Small delay for responsiveness
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

void loop()
{
  if (useRandomMode) {
    playRandomMode();
  } else {
    playSongMode();
  }
}