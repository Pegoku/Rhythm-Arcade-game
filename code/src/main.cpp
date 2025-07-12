#include <Arduino.h>
#include <FastLED.h>
#include <TM1637Display.h>

#define LED1_PIN 2
#define LED2_PIN 5

#define BUTTON1_pin 0
#define BUTTON2_pin 4

#define NUM_LEDS 5
#define BRIGHTNESS 20
#define LED_TYPE WS2812B

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];

int led1_pos=0;
int led2_pos=0;

int points1 = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(BUTTON1_pin, INPUT_PULLUP);
    pinMode(BUTTON2_pin, INPUT_PULLUP);

    FastLED.addLeds<LED_TYPE, LED1_PIN, GRB>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED2_PIN, GRB>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    Serial.println("Setup complete");
}

void loop() {
    // for (int i = 0; i < NUM_LEDS; i++) {
    //     fill_solid(leds1, NUM_LEDS, CRGB::Black);
    //     leds1[i] = CRGB::Red;
    //     FastLED.show();
    //     delay(500);
    //     fill_solid(leds2, NUM_LEDS, CRGB::Black);
    //     leds2[i] = CRGB::Blue;
    //     FastLED.show();
    //     delay(500);

    for (int i = 0; i < NUM_LEDS; i++) {
        leds1[i] = CRGB::Red;
        led1_pos = (i + 1);
        FastLED.show();
        
        static unsigned long lastTime = 0;
        static int last_i = -1;

        if (i != last_i) {
            lastTime = millis();
            last_i = i;
        }

        while (millis() - lastTime < 500) {
            if (digitalRead(BUTTON1_pin) == LOW) {
                if (led1_pos == 1) {
              points1 += 2;
                } else if (led1_pos == 2) {
              points1 += 1;
                } else {
              points1 -= 2;
                }
            }
            return;
        }
    }
}