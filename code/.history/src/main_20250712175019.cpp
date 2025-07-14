#include <Arduino.h>
#include <FastLED.h>
#include <TM1637Display.h>
#include "driver/i2s.h"
#include <math.h>

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
bool listenMode = true;    // Set to true for listen-only mode (plays song2 automatically)

// Song structure
struct Note {
  int stripIndex;      // 0-3 for strips, -1 for blank/pause
  unsigned long startTime;  // When this note starts (milliseconds from song start)
  unsigned long duration;   // How long this note lasts
};

// Tone frequencies for each button - better musical intervals
const int toneFreqs[4] = {523, 659, 784, 1047}; // C5, E5, G5, C6 (more musical)

// Map musical notes to button indices (improved mapping)
int getNoteButton(char note, bool isFlat = false) {
  switch(note) {
    case 'D': return 0;  // Low notes -> button 0 (C5 - 523Hz)
    case 'E': return 0;  // Low-mid notes -> button 0
    case 'F': return 1;  // Mid notes -> button 1 (E5 - 659Hz)
    case 'G': return 2;  // Mid-high notes -> button 2 (G5 - 784Hz)
    case 'A': return 2;  // High-mid notes -> button 2
    case 'B': return 3;  // High notes -> button 3 (C6 - 1047Hz)
    case 'C': return 1;  // Mid notes -> button 1
    default: return 0;
  }
}

// Define songs with timing
Note song1[] = {
  {0, 0,      10000},     // Red starts at 0ms, lasts 10000ms
  {1, 5000,   8000},    // Green starts at 500ms, lasts 8000ms (overlaps with red)
  {2, 12000,  6000},   // Blue starts at 12000ms, lasts 6000ms
  {3, 15000,  7000},   // Yellow starts at 15000ms, lasts 7000ms (overlaps with blue)
  {0, 25000,  5000},   // Red again at 25000ms
  {1, 28000,  6000},   // Green at 28000ms (overlaps with red)
  {2, 32000,  6000},   // Blue at 32000ms
  {3, 34000,  5000},   // Yellow at 3400ms
  {1, 36000,  7000},   // Green at 3600ms
  {0, 39000,  8000},   // Red at 3900ms
  {2, 42000,  5000},   // Blue at 4200ms
  {3, 44000,  6000},   // Yellow at 4400ms
  {1, 47000,  4000},   // Green at 4700ms
  {0, 49000,  7000},   // Red at 4900ms
  {-1, 50000, 5000},  // End marker/pause
};

Note song2[] = {
  // Measure 1: Bb G G D D A F F D (120 BPM = 500ms per quarter note)
  {getNoteButton('B'), 0, 250/2},     // Bb (eighth note)
  {getNoteButton('G'), 500, 500/2},   // G (quarter note)  
  {getNoteButton('G'), 1000, 250/2},  // G (eighth note)
  {getNoteButton('D'), 1250, 250/2},  // D (eighth note)
  {getNoteButton('D'), 1500, 250/2},  // D (eighth note)
  {getNoteButton('A'), 1750, 500/2},  // A (quarter note)
  {getNoteButton('F'), 2250, 500/2},  // F (quarter note)
  {getNoteButton('F'), 2750, 250/2},  // F (eighth note)
  {getNoteButton('D'), 3000, 500/2},  // D (quarter note)
  
  // Measure 2: D A F F C C E E F
  {getNoteButton('D'), 4000, 250/2},  // D (eighth note)
  {getNoteButton('A'), 4250, 500/2},  // A (quarter note)
  {getNoteButton('F'), 4750, 500/2},  // F (quarter note)
  {getNoteButton('F'), 5250, 250/2},  // F (eighth note)
  {getNoteButton('C'), 5500, 250/2},  // C (eighth note)
  {getNoteButton('C'), 5750, 250/2},  // C (eighth note)
  {getNoteButton('E'), 6000, 250/2},  // E (eighth note)
  {getNoteButton('E'), 6250, 250/2},  // E (eighth note)
  {getNoteButton('F'), 6500, 500/2},  // F (quarter note)
  
  // Measure 3: D Bb G G D D A F F D
  {getNoteButton('D'), 8000, 500},  // D (quarter note)
  {getNoteButton('B'), 8500, 250},  // Bb (eighth note)
  {getNoteButton('G'), 8750, 500},  // G (quarter note)
  {getNoteButton('G'), 9250, 250},  // G (eighth note)
  {getNoteButton('D'), 9500, 250},  // D (eighth note)
  {getNoteButton('D'), 9750, 250},  // D (eighth note)
  {getNoteButton('A'), 10000, 500}, // A (quarter note)
  {getNoteButton('F'), 10500, 500}, // F (quarter note)
  {getNoteButton('F'), 11000, 250}, // F (eighth note)
  {getNoteButton('D'), 11250, 500}, // D (quarter note)
  
  // Measure 4: D A F F C C E E F
  {getNoteButton('D'), 12000, 250}, // D (eighth note)
  {getNoteButton('A'), 12250, 500}, // A (quarter note)
  {getNoteButton('F'), 12750, 500}, // F (quarter note)
  {getNoteButton('F'), 13250, 250}, // F (eighth note)
  {getNoteButton('C'), 13500, 250}, // C (eighth note)
  {getNoteButton('C'), 13750, 250}, // C (eighth note)
  {getNoteButton('E'), 14000, 250}, // E (eighth note)
  {getNoteButton('E'), 14250, 250}, // E (eighth note)
  {getNoteButton('F'), 14500, 500}, // F (quarter note)
  
  // Measure 5: D Bb G G D D A F F D (half notes)
  {getNoteButton('D'), 16000, 1000},// D (half note)
  {getNoteButton('B'), 17000, 250}, // Bb (eighth note)
  {getNoteButton('G'), 17500, 1000},// G (half note)
  {getNoteButton('G'), 18500, 250}, // G (eighth note)
  {getNoteButton('D'), 18750, 250}, // D (eighth note)
  {getNoteButton('D'), 19000, 250}, // D (eighth note)
  {getNoteButton('A'), 19250, 500}, // A (quarter note)
  {getNoteButton('F'), 19750, 1000},// F (half note)
  {getNoteButton('F'), 20750, 250}, // F (eighth note)
  {getNoteButton('D'), 21000, 500}, // D (quarter note)
  
  // Measure 6: D A F F C C E E F
  {getNoteButton('D'), 22000, 250}, // D (eighth note)
  {getNoteButton('A'), 22250, 500}, // A (quarter note)
  {getNoteButton('F'), 22750, 1000},// F (half note)
  {getNoteButton('F'), 23750, 250}, // F (eighth note)
  {getNoteButton('C'), 24000, 500}, // C (quarter note)
  {getNoteButton('C'), 24500, 250}, // C (eighth note)
  {getNoteButton('E'), 24750, 500}, // E (quarter note)
  {getNoteButton('E'), 25250, 250}, // E (eighth note)
  {getNoteButton('F'), 25500, 1000},// F (half note)
  
  // Measure 7: D Bb G G D D A F F D (final measure)
  {getNoteButton('D'), 27000, 500}, // D (quarter note)
  {getNoteButton('B'), 27500, 250}, // Bb (eighth note)
  {getNoteButton('G'), 27750, 500}, // G (quarter note)
  {getNoteButton('G'), 28250, 250}, // G (eighth note)
  {getNoteButton('D'), 28500, 250}, // D (eighth note)
  {getNoteButton('D'), 28750, 250}, // D (eighth note)
  {getNoteButton('A'), 29000, 500}, // A (quarter note)
  {getNoteButton('F'), 29500, 1000},// F (half note)
  {getNoteButton('F'), 30500, 250}, // F (eighth note)
  {getNoteButton('D'), 30750, 1000},// D (half note)
  
  {-1, 32000, 1000}, // End marker
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

// I2S DAC configuration for MAX98357A
#define I2S_BCLK_PIN 26    // Bit clock pin
#define I2S_LRC_PIN  25    // Left/Right clock pin
#define I2S_DOUT_PIN 22    // Data out pin
#define SAMPLE_RATE  44100 // Sample rate
#define TONE_DURATION 200  // Tone duration in milliseconds

void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN,
    .ws_io_num = I2S_LRC_PIN,
    .data_out_num = I2S_DOUT_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void playTone(int frequency, int durationMs) {
  const int numSamples = (SAMPLE_RATE * durationMs) / 1000;
  int16_t *samples = (int16_t*)malloc(numSamples * 2 * sizeof(int16_t)); // Stereo
  
  if (samples == NULL) {
    Serial.println("Failed to allocate memory for audio samples");
    return;
  }
  
  // Generate sine wave
  for (int i = 0; i < numSamples; i++) {
    float time = (float)i / SAMPLE_RATE;
    int16_t sample = (int16_t)(sin(2.0 * PI * frequency * time) * 8192); // Reduced amplitude
    samples[i * 2] = sample;     // Left channel
    samples[i * 2 + 1] = sample; // Right channel
  }
  
  size_t bytesWritten;
  i2s_write(I2S_NUM_0, samples, numSamples * 2 * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  
  free(samples);
}

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
  if (listenMode || currentSong == 1) {
    currentSongArray = song2;
    currentSongLength = sizeof(song2) / sizeof(Note);
  } else {
    currentSongArray = song1;
    currentSongLength = sizeof(song1) / sizeof(Note);
  }

  display.setBrightness(0x0f);
  display.clear();
  display.showNumberDec(points1);
  
  // Initialize I2S DAC
  setupI2S();
  
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
        
        // Play tone for this button
        playTone(toneFreqs[stripIndex], TONE_DURATION);
        
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
        
        // Play tone for this button
        playTone(toneFreqs[note.stripIndex], TONE_DURATION);
        
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

void playListenMode() {
  // Initialize song if not active
  if (!songActive) {
    songStartTime = millis();
    songActive = true;
    Serial.println("Listen mode: Playing Crab Rave!");
  }

  unsigned long currentTime = millis() - songStartTime;
  
  // Clear all LEDs first
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  fill_solid(leds2, NUM_LEDS, CRGB::Black);
  fill_solid(leds3, NUM_LEDS, CRGB::Black);
  fill_solid(leds4, NUM_LEDS, CRGB::Black);

  bool anyNoteActive = false;
  
  // Check all notes to see which should be active now
  for (int i = 0; i < currentSongLength; i++) {
    Note note = currentSongArray[i];
    
    // Skip pause/end markers
    if (note.stripIndex == -1) {
      // Check if we've reached the end
      if (currentTime >= note.startTime) {
        songActive = false; // Restart song
        Serial.println("Crab Rave finished, restarting...");
        return;
      }
      continue;
    }
    
    // Check if this note should start playing
    if (currentTime >= note.startTime && currentTime <= (note.startTime + 50)) { // 50ms window for note start
      // Play the tone for this note
      playTone(toneFreqs[note.stripIndex], note.duration);
      Serial.print("Playing note: ");
      Serial.print(note.stripIndex);
      Serial.print(" at ");
      Serial.println(currentTime);
    }
    
    // Check if this note should be visually active
    if (currentTime >= note.startTime && currentTime < (note.startTime + note.duration)) {
      anyNoteActive = true;
      
      CRGB* currentLeds;
      CRGB activeColor;
      
      switch (note.stripIndex) {
        case 0:
          currentLeds = leds1;
          activeColor = CRGB::Red;
          break;
        case 1:
          currentLeds = leds2;
          activeColor = CRGB::Green;
          break;
        case 2:
          currentLeds = leds3;
          activeColor = CRGB::Blue;
          break;
        case 3:
          currentLeds = leds4;
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
      Serial.println("All notes completed, restarting Crab Rave...");
    }
  }
  
  delay(10); // Small delay for smooth operation
}

void loop()
{
  if (listenMode) {
    playListenMode();
  } else if (useRandomMode) {
    playRandomMode();
  } else {
    playSongMode();
  }
}