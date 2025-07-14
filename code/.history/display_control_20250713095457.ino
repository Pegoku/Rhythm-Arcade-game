#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_AlphaNum4 display2 = Adafruit_AlphaNum4();

char characters[4] = {'A', 'B', 'C', 'D'}; // Initial characters
int selectedIndex = 0; // Index of the currently selected character

const int buttonUp = 26;
const int buttonDown = 25;
const int buttonRight = 32;
const int buttonLeft = 33;

void setup() {
  Serial.begin(9600);
  display2.begin(0x70); // Initialize the display

  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonLeft, INPUT_PULLUP);

  updateDisplay();
}

void loop() {
  if (digitalRead(buttonUp) == LOW) {
    characters[selectedIndex]++;
    if (characters[selectedIndex] > 'Z') characters[selectedIndex] = 'A'; // Wrap around
    updateDisplay();
    delay(200); // Debounce delay
  }

  if (digitalRead(buttonDown) == LOW) {
    characters[selectedIndex]--;
    if (characters[selectedIndex] < 'A') characters[selectedIndex] = 'Z'; // Wrap around
    updateDisplay();
    delay(200); // Debounce delay
  }

  if (digitalRead(buttonRight) == LOW) {
    selectedIndex = (selectedIndex + 1) % 4; // Move to the next character
    updateDisplay();
    delay(200); // Debounce delay
  }

  if (digitalRead(buttonLeft) == LOW) {
    selectedIndex = (selectedIndex - 1 + 4) % 4; // Move to the previous character
    updateDisplay();
    delay(200); // Debounce delay
  }
}

void updateDisplay() {
  display2.clear();
  for (int i = 0; i < 4; i++) {
    display2.writeDigitAscii(i, characters[i]);
  }
  display2.writeDisplay();

  // Print the characters to the serial monitor
  Serial.print("Characters: ");
  for (int i = 0; i < 4; i++) {
    Serial.print(characters[i]);
  }
  Serial.println();
}
