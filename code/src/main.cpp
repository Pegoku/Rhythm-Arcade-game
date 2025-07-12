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

int led1_pos = 0;
int led2_pos = 0;

int points1 = 0;

TM1637Display display(16, 14);

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON1_pin, INPUT_PULLUP);
  pinMode(BUTTON2_pin, INPUT_PULLUP);

  FastLED.addLeds<LED_TYPE, LED1_PIN, GRB>(leds1, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, LED2_PIN, GRB>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  display.setBrightness(0x0f);
  display.clear();
  display.showNumberDec(points1);
  
  Serial.println("Setup complete");
}

void loop()
{
  int button1Pressed = 0;
  // Clear LEDs
  fill_solid(leds1, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // Last to First LED
  for (int idx = NUM_LEDS - 1; idx >= 0; idx--)
  {
    fill_solid(leds1, NUM_LEDS, CRGB::Black);
    leds1[idx] = CRGB::Red;
    FastLED.show();

    unsigned long startTime = millis();
    bool scored = false;

    // Wait for button press
    while (millis() - startTime < 200)
    {
      if (digitalRead(BUTTON1_pin) == LOW)
      {
        button1Pressed++;
        if (idx == 0)
        {
          points1 += 2;
        }
        else if (idx == 1)
        {
          points1 += 1;
        }
        else
        {
          points1 -= 2;
        }
        scored = true;
        break;
      }
      delay(10);
    }

    if (scored)
      break;
  }
  if (button1Pressed == 0)
  {
    points1 -= 2;
  }

  // Display points
  display.showNumberDec(points1);
  Serial.println(points1); 
  delay(1000);
}