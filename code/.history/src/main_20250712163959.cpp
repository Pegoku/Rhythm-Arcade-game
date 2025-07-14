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
  int stripIndex;      // 0-3 for strips, -1 for blank/pause
  unsigned long startTime;  // When this note starts (milliseconds from song start)
  unsigned long duration;   // How long this note lasts
};

// Define songs with timing
Note song1[] = {
  {0, 0, 1000},     // Red starts at 0ms, lasts 10000ms
  {1, 500, 800},    // Green starts at 5000ms, lasts 8000ms (overlaps with red)
  {2, 1200, 600},   // Blue starts at 12000ms, lasts 6000ms
  {3, 1500, 700},   // Yellow starts at 15000ms, lasts 7000ms (overlaps with blue)
  {0, 2500, 500},   // Red again at 25000ms
  {1, 2800, 600},   // Green at 28000ms (overlaps with red)
  {-1, 3500, 500},  // End marker/pause
};

Note song2[] = {
  {1, 0, 400},      // Green starts immediately
  {0, 200, 600},    // Red starts 200ms later (overlaps with green)
  {3, 400, 400},    // Yellow starts at 400ms
  {2, 600, 500},    // Blue starts at 600ms
  {0, 1200, 300},   // Quick red
  {1, 1300, 300},   // Quick green (almost simultaneous)
  {2, 1400, 300},   // Quick blue (almost simultaneous)
  {3, 1500, 300},   // Quick yellow (almost simultaneous)
  {-1, 2000, 500},  // End marker
};

// Song management
int currentSong = 0; // 0 for song1, 1 for song2
int currentNoteIndex = 0;
Note* currentSongArray;
int currentSongLength;

unsigned long songStartTime = 0;
bool songActive = false;

// Track button presses to avoid multiple scores per note
bool buttonStates[4] = {false, false, false, false};
bool buttonProcessed[4] = {false, false, false, false};
bool noteWasActive[4] = {false, false, false, false}; // Track if note was active in previous frame
bool noteWasPressed[4] = {false, false, false, false}; // Track if note was successfully pressed

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
  // Initialize song if not active
  if (!songActive) {
    songStartTime = millis();
    songActive = true;
    currentNoteIndex = 0;
    // Reset button processing
    for (int i = 0; i < 4; i++) {
      buttonProcessed[i] = false;
      noteWasActive[i] = false;
      noteWasPressed[i] = false;
    }
    Serial.println("Song started");
  }

  unsigned long currentTime = millis() - songStartTime;
  
  // Clear all LEDs first
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  fill_solid(leds3, NUM_LEDS, CRGB::Black);
  fill_solid(leds4, NUM_LEDS, CRGB::Black);

  bool anyNoteActive = false;
  bool noteCurrentlyActive[4] = {false, false, false, false}; // Track which notes are active this frame
  
  // Check all notes to see which should be active now
  for (int i = 0; i < currentSongLength; i++) {
    Note note = currentSongArray[i];
    
    // Skip pause/end markers
    if (note.stripIndex == -1) {
      // Check if we've reached the end
      if (currentTime >= note.startTime) {
        songActive = false; // Restart song
        Serial.println("Song finished, restarting...");
        return;
      }
      continue;
    }
    
    // Check if this note should be active
    if (currentTime >= note.startTime && currentTime < (note.startTime + note.duration)) {
      anyNoteActive = true;
      noteCurrentlyActive[note.stripIndex] = true;
      
      // Skip this note if it was already pressed successfully
      if (noteWasPressed[note.stripIndex]) {
        continue;
      }
      
      CRGB* currentLeds;
      uint8_t buttonPin;
      CRGB activeColor;
      
      switch (note.stripIndex) {
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
      
      // Calculate LED position based on note progress
      unsigned long noteProgress = currentTime - note.startTime;
      float progress = (float)noteProgress / note.duration;
      int ledIndex = NUM_LEDS - 1 - (int)(progress * NUM_LEDS);
      
      // Ensure ledIndex is within bounds
      if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
        currentLeds[ledIndex] = activeColor;
      }
      
      // Handle button input for this strip
      bool currentButtonState = (digitalRead(buttonPin) == LOW);
      bool previousButtonState = buttonStates[note.stripIndex];
      
      // Detect button press (transition from not pressed to pressed)
      if (currentButtonState && !previousButtonState && !buttonProcessed[note.stripIndex]) {
        buttonProcessed[note.stripIndex] = true;
        noteWasPressed[note.stripIndex] = true; // Mark this note as successfully pressed
        
        // Score based on LED position
        if (ledIndex == 0) {
          points1 += 10; // Perfect timing
          Serial.println("Perfect hit!");
        } else if (ledIndex == 1) {
          points1 += 5;  // Good timing
          Serial.println("Good hit!");
        } else if (ledIndex == 2) {
          points1 += 2;  // Okay timing
          Serial.println("Okay hit!");
        } else if (ledIndex >= 0) {
          points1 -= 1;  // Poor timing
          Serial.println("Poor timing!");
        }
        
        display.showNumberDec(points1);
        Serial.print("Score: ");
        Serial.println(points1);
      }
      
      buttonStates[note.stripIndex] = currentButtonState;
      
      // Reset button processing when button is released
      if (!currentButtonState) {
        buttonProcessed[note.stripIndex] = false;
      }
    }
  }
  
  // Check for notes that just ended without being pressed
  for (int i = 0; i < 4; i++) {
    // If note was active last frame but not this frame, and wasn't pressed
    if (noteWasActive[i] && !noteCurrentlyActive[i] && !noteWasPressed[i]) {
      points1 -= 5; // Penalty for missing note
      Serial.println("Missed note!");
      display.showNumberDec(points1);
      Serial.print("Score: ");
      Serial.println(points1);
    }
    
    // Update tracking
    noteWasActive[i] = noteCurrentlyActive[i];
    
    // Reset note pressed status when note ends
    if (!noteCurrentlyActive[i]) {
      noteWasPressed[i] = false;
      buttonProcessed[i] = false;
    }
  }
  
  FastLED.show();
  
  // If no notes are active and we haven't hit an end marker, restart
  if (!anyNoteActive && currentTime > 100) {
    // Check if we've passed all notes
    bool allNotesPassed = true;
    for (int i = 0; i < currentSongLength; i++) {
      if (currentSongArray[i].stripIndex != -1) {
        if (currentTime < (currentSongArray[i].startTime + currentSongArray[i].duration)) {
          allNotesPassed = false;
          break;
        }
      }
    }
    
    if (allNotesPassed) {
      songActive = false; // Restart song
      Serial.println("All notes completed, restarting...");
    }
  }
  
  delay(10); // Small delay for smooth operation
}

void loop()
{
  if (useRandomMode) {
    playRandomMode();
  } else {
    playSongMode();
  }
}