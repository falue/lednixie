// Libraries
#include "FastLED.h"

// User defined -----------------------------------------------------------------------------------
// LEDS & COLORS
const int numOfDigits = 7;        // Number of physical LEDs DIgits
int brightness = 255;             // General brightness 0-255. 0 = off, 255 = max.
CRGB regularColor = CRGB::White;  // Define colors of LEDs while number is static. Does not change color if pauseWhenReached = 0
CRGB plusColor = CRGB::Lime;      // Define colors of LEDs while adding to number. Standard: "Green" or "Lime"
CRGB minusColor = CRGB::Red;      // Define colors of LEDs while adding to number.  Standard: "Red"
                                  //   See all possible color names (due to color conversions, colors do not really match the screen):
                                  //   https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list

// BEHAVIOR
#define alignRight                // Make numbers align right. Comment-out to align left.
#define startNumber 1234567       // Set fixed number when start up. Comment-out when this should be set randomly to 0-9999999.
int speed = 100;                  // Speed (ms) of number changing. => Smaller number, faster action! Sweetspot 100.
// #define randomSpeed 600        // Random pauses up to 0-X ms between number changing. => Less "robotic" number changes.
                                  //   Comment-out to not use this. Sweetspot 600.
// #define randomFluct 10         // Every now an then, change main number a bit. Comment-out to not change number randomly.
                                  //   Number is frequency of random fluctuation: Chance of 1 in X. Bigger number = lower frequency. Sweetspot 200.
// #define randomProportional     // If defined, randomFluct jumps proportional to size of total number.
                                  //   => The bigger current number, the bigger the jumps. Comment out to randomly jump only -1 or +1.
int randomBias = 0;               // Change to change main number to be generally increasing or decreasing. Percent from -100 to 100:
                                  //   -100 to -1: Biased towards more lower random numbers. Values under -100 yield only lower numbers.
                                  //   0: equally lower and higher random numbers (no bias).
                                  //   1 to 100: Biased towards more higher random numbers. Values above 100 yield only higher numbers.
int pauseWhenReached = 0;         // Keep red/green colors this long (ms) when reached number before changing back
                                  //   to standard color. During this time no changes can occur.
                                  //   Set to 0 for no static pause (=> no change of color). Sweetspot 1500.
                                  
// ATTENTION
// no green zero, and no red 999 999.. possible!

// ------------------------------------------------------------------------------------------------


// Do not touch -----------------------------------------------------------------------------------
#define LED_PIN 6
#define LED_TYPE  WS2812
#define COLOR_ORDER  GRB
#define NUM_LEDS numOfDigits*10
CRGB leds[NUM_LEDS];

long randomStartpoint = 0;  // gets randomized in setup()
long currentNumber = randomStartpoint;
long numberToDisplay = randomStartpoint;
boolean reached = false;
long maxNumber;
// ------------------------------------------------------------------------------------------------


// Setup ------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  delay(3000); // 3 second delay for recovery
  Serial.println("*********************************");

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  randomSeed(analogRead(0));  // make random() more random

  maxNumber = pow(10, numOfDigits);

  #ifndef startNumber
    randomStartpoint = random(0, maxNumber);
    currentNumber = randomStartpoint;
    numberToDisplay = randomStartpoint;
  #else
    currentNumber = startNumber;
    numberToDisplay = startNumber;
  #endif

  FastLED.setBrightness(brightness);
  
  // Display the start number
  displayNumber(currentNumber, plusColor);
  FastLED.show();
  
  Serial.println("Startup complete.");
  Serial.println("*********************************");
  Serial.println("Enter any number in the field above and press enter.");
  Serial.println("Press enter with empty field AGAIN to skip counting up");
  Serial.println("  or down and jump directly to number without counting up/down.");
  Serial.println("Enter any number like +5 or -45 to increase or decrease the current number.");  
}


// Loop -------------------------------------------------------------------------------------------
void loop() {
  // make random changes to number.
  #ifdef randomFluct
    // Get a random number from 0 to randomFrequency. if its below 1 (null) then make a random change to the numbers.
    if(random(0, randomFluct) < 1) {
      Serial.println("> ACTION: Auto-random.");
      #ifdef randomProportional
        long sqrtOfNumber = sqrt(numberToDisplay)*100;  // *100 because otherwise floating point necessity, which sucks
        int minimal = (sqrtOfNumber*-1 - (sqrtOfNumber*-1 / 100 * randomBias))/100;
        int maximal = (sqrtOfNumber + (sqrtOfNumber / 100 * randomBias))/100;
        numberToDisplay += random(minimal, maximal);
      #else
        numberToDisplay += random(0+randomBias, 200+randomBias) < 100 ? -1 : 1;
      #endif  
      reached = false;
    }
  #endif

  // wait for user input
  // Serial.println("Waiting for user input: Enter for next LED, or number of LED");
  while (Serial.available()) {
    // Try to convert input to number to 
    String serialReading = Serial.readString();
    if (isValidNumber(serialReading)) {
      // add or substract if entered "+5" or "-14"
      if(serialReading.charAt(0) == '+' || serialReading.charAt(0) == '-') {  // single quotes for comparison of chars!
        Serial.println("> ACTION: Add entered number to current number");
        numberToDisplay += serialReading.toInt();
        
      } else {
        Serial.println("> ACTION:  Get to entered number.");
        numberToDisplay = serialReading.toInt();
      }
      
    } else if(serialReading.length() == 1) {
      Serial.println("> ACTION: Empty text: jump directly to target number");
      // set color depending on "losing" or "winning"
      CRGB colorJump = numberToDisplay > currentNumber ? plusColor : minusColor;
      currentNumber = numberToDisplay;
      // Black out all LEDs
      fill_solid(leds, NUM_LEDS, CRGB::Black);  
      // Set the LED array for the currentNumber
      displayNumber(numberToDisplay, colorJump);
      reached = true;      
      // Display the needed LEDs
      FastLED.show();
      
    } else {
      Serial.print("> ACTION: Not a number. Auto increment counter once.");
      Serial.println("Leave empty to jump directly or enter number to counter effect.");
      numberToDisplay++;
    }
    
    reached = false;  // TTHIS IS WHAT I DONE - does it work? does numbers stay white when entering random text?
    Serial.flush();   // wait and terminate readString() action
  }

  // Do not go under 0 and not higher than the digits allow (10^6)
  if(numberToDisplay < 0) {
    numberToDisplay = 0;
  }
  if(numberToDisplay >= maxNumber) {
    numberToDisplay = maxNumber - 1;
  }

  // If current number is not yet the desired number, add or subtract to got there.
  if(currentNumber != numberToDisplay ) {
    // black out all first to clear the display.
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // set color depending on "losing" or "winning"
    CRGB color = numberToDisplay > currentNumber ? plusColor : minusColor;

    if(numberToDisplay > currentNumber) {
      // currentNumber++;  // linear
      currentNumber += sqrt(numberToDisplay - currentNumber);
    } else {
      // currentNumber--;  // linear
      currentNumber -= sqrt(currentNumber - numberToDisplay);
    }
    
    // Set the LED array for the currentNumber
    displayNumber(currentNumber, color);

    // Display the needed LEDs
    FastLED.show();
    Serial.println(currentNumber);
    
  } else if(currentNumber == numberToDisplay && !reached && pauseWhenReached > 0) {
    // Mark reached
    //Serial.print(currentNumber);
    Serial.println("----------");
    
    // Wait to show green/red number first
    delay(pauseWhenReached);
    
    // Black out all LEDs
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Set the LED array for the currentNumber
    displayNumber(numberToDisplay, regularColor);
    reached = true;
    
    // Display the needed LEDs
    FastLED.show();
//  } else {
//    // has reached but maybe....????
//    Serial.println("else...");    
  }



  #ifdef randomSpeed
    delay(random(1, randomSpeed));
  #else 
    delay(speed);  // linear
  #endif
}


// General functions ------------------------------------------------------------------------------
void displayNumber(long numberToDisplay, CRGB color) {
  // Serial.print("Displaying number:\t");
  // Serial.println(numberToDisplay);
  
  // split numberToDisplay to digits.
  int one = numberToDisplay >= 0 ? (numberToDisplay / 1U) % 10 : -1;
  int ten = numberToDisplay > 9 ? (numberToDisplay / 10U) % 10 : -1;
  int hundred = numberToDisplay > 99 ? (numberToDisplay / 100U) % 10 : -1;
  int thousand = numberToDisplay > 999 ? (numberToDisplay / 1000U) % 10 : -1;
  int tenthousand = numberToDisplay > 9999 ? (numberToDisplay / 10000U) % 10 : -1;
  int hundredthousand = numberToDisplay > 99999 ? (numberToDisplay / 100000U) % 10 : -1;
  int million = numberToDisplay > 999999 ? (numberToDisplay / 1000000U) % 10 : -1;

//  Serial.print("million: ");
//  Serial.print(million);
//  Serial.print(", hundredthousand: ");
//  Serial.print(hundredthousand);
//  Serial.print(", tenthousand: ");
//  Serial.print(tenthousand);
//  Serial.print(", thousand: ");
//  Serial.print(thousand);
//  Serial.print(", hundred: ");
//  Serial.print(hundred);
//  Serial.print(", ten: ");
//  Serial.print(ten);
//  Serial.print(", one: ");
//  Serial.print(one);
//  Serial.println("");

  int totalDigits = countDigits(numberToDisplay);

  #ifdef alignRight
    totalDigits += numOfDigits-totalDigits;
  #endif

  if(one >= 0) {  // always display zero! even if no other number
    displayDigit(one, (totalDigits-1)*10, color);
  }
  if(ten >= 0) {
    displayDigit(ten, (totalDigits-2)*10, color);
  }
  if(hundred >= 0) {
    displayDigit(hundred, (totalDigits-3)*10, color);
  }
  if(thousand >= 0) {
    displayDigit(thousand, (totalDigits-4)*10, color);
  }
  if(tenthousand >= 0) {
    displayDigit(tenthousand, (totalDigits-5)*10, color);
  }
  if(hundredthousand >= 0) {
    displayDigit(hundredthousand, (totalDigits-6)*10, color);
  }
  if(million >= 0) {
    displayDigit(million, (totalDigits-7)*10, color);
  }
}


void displayDigit(int digit, int offset, CRGB color) {    
  float ledId = offset;

  if(digit % 2 == 0) {
    ledId += digit/2+5;
  } else {
    ledId += (digit-1)/2;
  }
  
  leds[(int) ledId] = color;
}


boolean isValidNumber(String str) {
  boolean isNum = false;
  str.trim();
  for (byte i = 0; i < str.length(); i++) {
    isNum = isDigit(str.charAt(i)) || str.charAt(i) == '+' || str.charAt(i) == '.' || str.charAt(i) == '-';
    if (!isNum) {
      return false;
    }
  }
  return isNum;
}


byte countDigits(int number){
  byte count=0;
  while(number){
    number=number/10;
    count++;
  }
  return count;
}
