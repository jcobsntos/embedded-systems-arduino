const int ledPins[] = {A0, A1, A2, A3, A4, A5};
const int segmentPins[] = {11, 10, 2, 5, 4, 7, 6};
const int dpPin = 3, digitPins[] = {8, 9}, buttonPin = 12;
const byte digitPatterns[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

unsigned long previousMillis = 0;
int currentLedIndex = 0, hourCount = 0;
bool buttonPressed = false, resetState = false, allOffState = false;
bool ascendingDirection = true;  

enum CountingMode { ASCENDING, DESCENDING, ODD_ONLY, EVEN_ONLY, ASCEND_DESCEND };
CountingMode mode = ASCENDING; 

void setup() {
  for (int i = 0; i < 6; i++) pinMode(ledPins[i], OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);
  pinMode(dpPin, OUTPUT);
  pinMode(digitPins[0], OUTPUT);
  pinMode(digitPins[1], OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  digitalWrite(ledPins[currentLedIndex], HIGH);
  displayNumber(hourCount);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle LED sequencing
  if (currentMillis - previousMillis >= 500 && !resetState && !allOffState) {
    previousMillis = currentMillis;
    if (++currentLedIndex >= 6) { 
      allOffState = true; 
      updateLeds(false); 
      updateHour(); 
    } else {
      digitalWrite(ledPins[currentLedIndex], HIGH);
    }
  }

  // Handle reset state
  if (allOffState && currentMillis - previousMillis >= 500) {
    allOffState = false; currentLedIndex = 0;
    digitalWrite(ledPins[currentLedIndex], HIGH);
    previousMillis = currentMillis;
  }

  // Handle button press for reset
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    delay(50); // Debouncing
    if (digitalRead(buttonPin) == LOW) {
      buttonPressed = true; 
      resetHourglass();
      for (int i = 0; i < 6; i++) {
        digitalWrite(ledPins[i], LOW);
      }
      currentLedIndex = 0;
      resetState = true;
      previousMillis = currentMillis;
    }
  }
  
  if (digitalRead(buttonPin) == HIGH && buttonPressed) buttonPressed = false;

  if (resetState && currentMillis - previousMillis >= 500) {
    resetState = false;
    digitalWrite(ledPins[currentLedIndex], HIGH);
    previousMillis = currentMillis;
  }

  multiplexDisplay();
}

// Function to update LEDs on/off
void updateLeds(bool state) {
  for (int i = 0; i < 6; i++) digitalWrite(ledPins[i], state ? HIGH : LOW);
}

void updateHour() {
  switch (mode) {
    case ASCENDING:
      hourCount += 1;  
      if (hourCount >= 24) hourCount = 0; 
      break;

    case DESCENDING:
      hourCount -= 1;  
      if (hourCount < 0) hourCount = 23; 
      break;

    case ODD_ONLY:
      if (hourCount % 2 == 0) {
        hourCount += 1;  
      }
      hourCount += 2;  
      if (hourCount >= 24) hourCount = 1; 
      break;

    case EVEN_ONLY:
      if (hourCount % 2 != 0) {
        hourCount -= 1;  
      }
      hourCount += 2; 
      if (hourCount >= 24) hourCount = 0; 
      break;

    case ASCEND_DESCEND:
      if (ascendingDirection) {
        hourCount += 1;  
        if (hourCount >= 10) {  
          hourCount = 10; 
          ascendingDirection = false; 
        }
      } else {
        hourCount -= 1;  
        if (hourCount <= 0) {  
          hourCount = 0; 
          ascendingDirection = true; 
        }
      }
      break;
  }

  displayNumber(hourCount); 
}

int currentMode = mode;  

void resetHourglass() {
  updateLeds(false);
  currentLedIndex = 0;

  if (currentMode == ASCENDING || currentMode == EVEN_ONLY) {
    hourCount = 0; 
  } else if (currentMode == ODD_ONLY) {
    hourCount = 1; 
  } else if (currentMode == DESCENDING) {
    hourCount = 23; 
  } else if (currentMode == ASCEND_DESCEND) {
    hourCount = 0; 
    ascendingDirection = true;  
  }
  displayNumber(hourCount);
  resetState = true;
  previousMillis = millis();
}

// Function to display a two-digit number on the 7-segment display
void displayNumber(int number) {
  displayDigit(number / 10, 0); // Tens digit
  displayDigit(number % 10, 1); // Ones digit
}

// Function to display a single digit on one of the 7-segment positions
void displayDigit(int digit, int position) {
  digitalWrite(digitPins[0], LOW);
  digitalWrite(digitPins[1], LOW);
  for (int i = 0; i < 7; i++) digitalWrite(segmentPins[i], bitRead(digitPatterns[digit], i));
  digitalWrite(digitPins[position], HIGH);
}

// Multiplexing display to show both digits
void multiplexDisplay() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate >= 5) {
    lastUpdate = millis();
    static bool showTens = true;
    displayDigit(showTens ? hourCount / 10 : hourCount % 10, showTens ? 0 : 1);
    showTens = !showTens;
  }
}

