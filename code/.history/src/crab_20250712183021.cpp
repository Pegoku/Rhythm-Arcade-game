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

// Crab Rave melody notes (simplified version)
// Using MIDI note numbers converted to frequencies
const float crab_rave_melody[] = {
  261.63, 293.66, 329.63, 261.63, 293.66, 329.63, 261.63, 293.66,
  329.63, 392.00, 329.63, 293.66, 261.63, 293.66, 329.63, 392.00,
  440.00, 392.00, 329.63, 293.66, 261.63, 329.63, 392.00, 440.00,
  493.88, 440.00, 392.00, 329.63, 293.66, 261.63, 293.66, 329.63
};

const int melody_length = sizeof(crab_rave_melody) / sizeof(crab_rave_melody[0]);
const float note_duration = 0.3; // seconds per note

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
    
    // Generate sine wave with some harmonics for richer sound
    float sample = 0.3 * sin(2 * PI * frequency * time) +
                   0.2 * sin(2 * PI * frequency * 2 * time) +
                   0.1 * sin(2 * PI * frequency * 3 * time);
    
    // Apply envelope (attack, sustain, release)
    float envelope = 1.0;
    float attack_time = 0.05;
    float release_time = 0.1;
    
    if (time < attack_time) {
      envelope = time / attack_time;
    } else if (time > duration - release_time) {
      envelope = (duration - time) / release_time;
    }
    
    sample *= envelope;
    
    // Convert to 16-bit and apply to both channels
    int16_t sample_16 = (int16_t)(sample * 16383);
    buffer[i * 2] = sample_16;     // Left channel
    buffer[i * 2 + 1] = sample_16; // Right channel
  }
  
  // Fill remaining buffer with silence if needed
  for (int i = samples_to_generate * 2; i < buffer_size; i++) {
    buffer[i] = 0;
  }
}

void play_crab_rave() {
  Serial.println("🦀 CRAB RAVE TIME! 🦀");
  
  for (int note = 0; note < melody_length; note++) {
    float frequency = crab_rave_melody[note];
    
    // Generate audio for this note
    generate_tone(frequency, note_duration, audio_buffer, BUFFER_SIZE);
    
    // Send to I2S
    size_t bytes_written;
    i2s_write(I2S_PORT, audio_buffer, BUFFER_SIZE * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    Serial.printf("Playing note %d: %.2f Hz\n", note + 1, frequency);
  }
  
  Serial.println("🦀 Crab Rave complete! 🦀");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Crab Rave Player Starting...");
  
  setup_i2s();
  Serial.println("I2S initialized");
  
  delay(1000);
}

void loop() {
  play_crab_rave();
  delay(2000); // Wait 2 seconds before playing again
}