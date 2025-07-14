#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// I2S configuration
#define I2S_WS_PIN      25  // Word Select (LRCLK)
#define I2S_SCK_PIN     26  // Serial Clock (BCLK)
#define I2S_SD_PIN      22  // Serial Data (DIN)
#define I2S_PORT        I2S_NUM_0
#define SAMPLE_RATE     44100
#define BITS_PER_SAMPLE 16
#define CHANNELS        2

// Audio buffer
#define BUFFER_SIZE     1024
int16_t audio_buffer[BUFFER_SIZE];

// Note frequencies in Hz (Bb major scale, 4th octave)
#define REST    0.0
#define Bb4     466.16
#define C5      523.25
#define D5      587.33
#define Eb5     622.25
#define F5      698.46
#define G5      783.99
#define A5      880.00

// Crab Rave melody from the sheet music
// Each line represents one measure, tempo = 120 BPM
struct Note {
  float frequency;
  float duration; // in beats (quarter note = 1.0)
};

// Based on the sheet music provided
const Note crab_rave_melody[] = {
  // Measure 1: Bb G G D D A F F D
  {Bb4, 0.5}, {G5, 0.5}, {G5, 0.5}, {D5, 0.25}, {REST, 0.25}, {D5, 0.5}, {A5, 0.5}, {F5, 0.5}, {F5, 0.5}, {D5, 0.5},
  
  // Measure 2: D A F F C C E E F
  {REST, 0.25}, {D5, 0.25}, {A5, 0.5}, {F5, 0.5}, {F5, 0.5}, {C5, 0.25}, {REST, 0.25}, {C5, 0.5}, {Eb5, 0.25}, {REST, 0.25}, {Eb5, 0.5}, {F5, 0.5},
  
  // Measure 3: D Bb G G D D A F F D
  {D5, 0.5}, {Bb4, 0.5}, {G5, 0.5}, {G5, 0.5}, {D5, 0.25}, {REST, 0.25}, {D5, 0.5}, {A5, 0.5}, {F5, 0.5}, {F5, 0.5}, {D5, 0.5},
  
  // Measure 4: D A F F C C E E F
  {REST, 0.25}, {D5, 0.25}, {A5, 0.5}, {F5, 0.5}, {F5, 0.5}, {C5, 0.25}, {REST, 0.25}, {C5, 0.5}, {Eb5, 0.25}, {REST, 0.25}, {Eb5, 0.5}, {F5, 0.5},
  
  // Measure 5: D Bb G G D D A F F D
  {D5, 1.0}, {Bb4, 0.5}, {G5, 0.5}, {G5, 1.0}, {D5, 0.25}, {REST, 0.25}, {D5, 0.5}, {A5, 0.5}, {F5, 1.0}, {F5, 0.5}, {D5, 0.5},
  
  // Measure 6: D A F F C C E E F
  {REST, 0.25}, {D5, 0.25}, {A5, 0.5}, {F5, 1.0}, {F5, 0.5}, {C5, 0.25}, {REST, 0.25}, {C5, 1.0}, {Eb5, 0.25}, {REST, 0.25}, {Eb5, 0.5}, {F5, 0.5},
  
  // Measure 7: D Bb G G D D A F F D
  {D5, 1.0}, {Bb4, 0.5}, {G5, 0.5}, {G5, 1.0}, {D5, 0.25}, {REST, 0.25}, {D5, 0.5}, {A5, 0.5}, {F5, 1.0}, {F5, 0.5}, {D5, 0.5}
};

const int melody_length = sizeof(crab_rave_melody) / sizeof(crab_rave_melody[0]);
const float bpm = 120.0;
const float beat_duration = 60.0 / bpm; // Duration of one beat in seconds

void setup_i2s() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK_PIN,
    .ws_io_num = I2S_WS_PIN,
    .data_out_num = I2S_SD_PIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
}

void generate_tone(float frequency, float duration, int16_t* buffer, int buffer_size) {
  int samples_per_note = (int)(SAMPLE_RATE * duration);
  int samples_to_generate = min(samples_per_note, buffer_size / 2); // Divide by 2 for stereo
  
  for (int i = 0; i < samples_to_generate; i++) {
    float time = (float)i / SAMPLE_RATE;
    float sample = 0.0;
    
    if (frequency > 0) { // Not a rest
      // Generate tone with harmonics for electronic music feel
      sample = 0.4 * sin(2 * PI * frequency * time) +
               0.3 * sin(2 * PI * frequency * 2 * time) +
               0.2 * sin(2 * PI * frequency * 3 * time) +
               0.1 * sin(2 * PI * frequency * 4 * time);
      
      // Apply envelope (attack, sustain, release)
      float envelope = 1.0;
      float attack_time = 0.02;
      float release_time = duration * 0.2;
      
      if (time < attack_time) {
        envelope = time / attack_time;
      } else if (time > duration - release_time) {
        envelope = (duration - time) / release_time;
      }
      
      sample *= envelope;
    }
    
    // Convert to 16-bit and apply to both channels
    int16_t sample_16 = (int16_t)(sample * 12000); // Reduce volume slightly
    buffer[i * 2] = sample_16;     // Left channel
    buffer[i * 2 + 1] = sample_16; // Right channel
  }
  
  // Fill remaining buffer with silence if needed
  for (int i = samples_to_generate * 2; i < buffer_size; i++) {
    buffer[i] = 0;
  }
}

void play_note(float frequency, float beats) {
  float duration = beats * beat_duration;
  int samples_needed = (int)(SAMPLE_RATE * duration);
  int buffer_samples = BUFFER_SIZE / 2; // Account for stereo
  
  // Play note in chunks if it's longer than buffer
  while (samples_needed > 0) {
    int chunk_duration = min(samples_needed, buffer_samples);
    float chunk_time = (float)chunk_duration / SAMPLE_RATE;
    
    generate_tone(frequency, chunk_time, audio_buffer, BUFFER_SIZE);
    
    size_t bytes_written;
    i2s_write(I2S_PORT, audio_buffer, chunk_duration * 2 * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    samples_needed -= chunk_duration;
  }
}

void play_crab_rave() {
  Serial.println("🦀 CRAB RAVE TIME! 🦀");
  Serial.printf("Playing at %d BPM\n", (int)bpm);
  
  for (int note = 0; note < melody_length; note++) {
    float frequency = crab_rave_melody[note].frequency;
    float beats = crab_rave_melody[note].duration;
    
    if (frequency > 0) {
      Serial.printf("Note %d: %.2f Hz for %.2f beats\n", note + 1, frequency, beats);
    } else {
      Serial.printf("Note %d: REST for %.2f beats\n", note + 1, beats);
    }
    
    play_note(frequency, beats);
  }
  
  Serial.println("🦀 Crab Rave complete! 🦀");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Crab Rave Player Starting...");
  Serial.println("Using accurate sheet music transcription!");
  
  setup_i2s();
  Serial.println("I2S initialized");
  
  delay(1000);
}

void loop() {
  play_crab_rave();
  delay(3000); // Wait 3 seconds before playing again
}