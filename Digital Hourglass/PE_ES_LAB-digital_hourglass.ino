// Define LED pins (hourglass effect)
const int ledPins[] = {8, 9, 10, 11, 12, 13};  // Array of LED pins (6 LEDs)

// Define 7-segment display pins (a-g, dp)
const int segmentPins[] = {2, 3, 4, 5, 6, A0, A1};  // Pins for segments a-g
const int dpPin = A2;  // Pin for decimal point

// Define the control pins for the two digits (common cathode for each 7-segment digit)
const int digitPins[] = {A3, A4};  // Control pins for the tens and ones digits

const int buttonPin = 7;  // Pin for reset button
const long interval = 500;  // 2 seconds per LED in milliseconds

unsigned long previousMillis = 0;
int currentLedIndex = 0;  // Index for the currently active LED
int hourCount = 0;  // Count for hours (00 to 23)
bool buttonPressed = false;  // Flag to track button state
bool resetState = false;  // Flag to indicate reset state
bool allOffState = false;  // Flag to indicate if all LEDs should be off before restarting

// Array of digit patterns for 7-segment display (common cathode)
const byte digitPatterns[10] = {
  0b00111111,  // 0
  0b00000110,  // 1
  0b01011011,  // 2
  0b01001111,  // 3
  0b01100110,  // 4
  0b01101101,  // 5
  0b01111101,  // 6
  0b00000111,  // 7
  0b01111111,  // 8
  0b01101111   // 9
};

void setup() {
  // Initialize the LED pins as outputs
  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Ensure all LEDs are off initially
  }

  // Initialize the segment pins as outputs
  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  pinMode(dpPin, OUTPUT);
  digitalWrite(dpPin, LOW);  // Ensure decimal point is off

  // Initialize the digit control pins as outputs
  pinMode(digitPins[0], OUTPUT);
  pinMode(digitPins[1], OUTPUT);

  // Initialize the button pin as input with internal pullup resistor
  pinMode(buttonPin, INPUT_PULLUP);

  // Turn on the first LED to start the hourglass (pin 8)
  digitalWrite(ledPins[currentLedIndex], HIGH);

  // Initialize the display to 00
  displayNumber(hourCount);
}

void loop() {
  // Get the current time in milliseconds
  unsigned long currentMillis = millis();

  // Check if the interval has passed and we're not in reset or all-off state
  if (currentMillis - previousMillis >= interval && !resetState && !allOffState) {
    // Save the current time for the next interval
    previousMillis = currentMillis;

    // Move to the next LED in sequence
    currentLedIndex++;

    // If all LEDs have been lit, move to all-off state
    if (currentLedIndex >= 6) {
      allOffState = true;
      // Turn off all LEDs before restarting
      for (int i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], LOW);
      }

      // Increment hour count and display on 7-segment
      hourCount++;
      if (hourCount >= 24) {
        hourCount = 0;  // Reset to 00 after 23
      }
      displayNumber(hourCount);

      // Reset the timer to control the delay before restarting
      previousMillis = currentMillis;
    } else {
      // Turn on the current LED (and all previous LEDs remain on)
      digitalWrite(ledPins[currentLedIndex], HIGH);
    }
  }

  // Handle the all-off state before restarting the sequence
  if (allOffState && currentMillis - previousMillis >= interval) {
    // Exit the all-off state and restart the sequence
    allOffState = false;
    currentLedIndex = 0;  // Reset to the first LED
    digitalWrite(ledPins[currentLedIndex], HIGH);  // Turn on pin 8

    // Reset the timer to continue normal timing
    previousMillis = currentMillis;
  }

  // Check if the reset button is pressed (LOW state due to pullup resistor)
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    // Button debounce - wait a few milliseconds to avoid multiple resets
    delay(50);

    // If the button is still pressed, reset the hourglass
    if (digitalRead(buttonPin) == LOW) {
      buttonPressed = true;  // Mark the button as pressed

      // Turn off all LEDs
      for (int i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], LOW);
      }

      // Reset the current LED index and hour count
      currentLedIndex = 0;
      hourCount = 0;

      // Reset the display to 00
      displayNumber(hourCount);

      // Mark the system in reset state
      resetState = true;

      // Reset the timer to start counting for the delay
      previousMillis = currentMillis;
    }
  }

  // Detect when the button is released (HIGH state)
  if (digitalRead(buttonPin) == HIGH && buttonPressed) {
    buttonPressed = false;  // Reset button press state
  }

  // After the reset, wait for the interval before turning on pin 8
  if (resetState && (currentMillis - previousMillis >= interval)) {
    // Turn on pin 8 (index 0) after the delay
    digitalWrite(ledPins[currentLedIndex], HIGH);

    // Exit reset state and resume normal operation
    resetState = false;

    // Reset the timer to continue normal timing
    previousMillis = currentMillis;
  }

  // Multiplex the display between tens and ones digits
  multiplexDisplay();
}

// Function to display a two-digit number on the dual 7-segment display
void displayNumber(int number) {
  int tens = number / 10;  // Extract tens digit
  int ones = number % 10;  // Extract ones digit

  // Update the displayed digits
  displayDigit(tens, 0);  // Display tens digit on the first display
  displayDigit(ones, 1);  // Display ones digit on the second display
}

// Function to display a single digit (0-9) on one 7-segment display
void displayDigit(int digit, int position) {
  byte pattern = digitPatterns[digit];  // Select the correct digit pattern

  // Turn off both digits
  digitalWrite(digitPins[0], LOW);
  digitalWrite(digitPins[1], LOW);

  // Assign the segments based on the pattern for the given digit
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], bitRead(pattern, i));
  }

  // Turn on the correct digit (select which 7-segment display to light up)
  digitalWrite(digitPins[position], HIGH);
}

// Function to rapidly alternate between tens and ones digits (multiplexing)
void multiplexDisplay() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  if (now - lastUpdate >= 5) {  // Update every 5 milliseconds
    lastUpdate = now;

    // Alternate between displaying the tens and ones digits
    static bool showTens = true;
    if (showTens) {
      displayDigit(hourCount / 10, 0);  // Show tens digit
    } else {
      displayDigit(hourCount % 10, 1);  // Show ones digit
    }
    showTens = !showTens;  // Toggle between tens and ones
  }
}
