// Lifeclock timer by Christian M. Restifo
// Version 0.1a
// November 30, 2021
// No license. Use as you wish without any restrictions.
// "Fish, and plankton. And sea greens, and protein from the sea. It's all here, ready. Fresh as harvest day."

// This sketch keeps track of time and changes the color of a neopixel as necessary to denote the passage of time.
// Pressing the button will reset the timer

// Change the following values to change the days between different states.
// NOTE: We use unsigned long for time, which means it will roll over after 49.7 days. While this code can handle
// the rollover, it will not work for time spans greater than 49.7 days. In other words, you can't set the maximum time
// until red (including yellow time) greater than 49.7 days.

#define DAYS_UNTIL_YELLOW 4
#define DAYS_UNTIL_RED    7
#define DAYS_UNTIL_FLASH  14

// The following values set the pins for the Neopixel and the reset button

#define LED_PIN 6
#define RESET_PIN 10

// Change this value to change the delay after pixel data is sent. You shouldn't have to change this unless you get weird behavior.

#define DELAY 10

// The following are set to determine how fast the led will flash red. Values are in milliseconds.

#define FLASH_ON 250
#define FLASH_OFF 250

// we have to declare these at unsigned long or we get math problems due to multiplying ints

unsigned long a = 24;
unsigned long b = 60;
unsigned long c = 60;
unsigned long d = 1000;

unsigned long yellow_time = DAYS_UNTIL_YELLOW * a * b * c *d ; // 24 hours in a day, 60 minutes in an hour, 60 seconds in a minute, 1000 milliseconds in a second
unsigned long red_time = DAYS_UNTIL_RED * a * b * c * d; // Same conversion as above
unsigned long flash_time = DAYS_UNTIL_FLASH * a * b * c * d; // Same conversion as above

//Swap out these values below with the ones above if you want to test things without waiting days
//unsigned long yellow_time = 4000;
//unsigned long red_time = 10000;
//unsigned long flash_time = 15000;

unsigned long start_time = 0; // time at the last button push
unsigned long current_time = 0; // time since last button push
unsigned long red_flash_time = 0; // timer for each flashing state (on/off)
boolean flash_flag = false; // used to keep track of flash status. true is on, false is off
int color_mode = 0; // keeps track of which color mode we're in. 0 = green, 1 = yellow, 2 = red, 3 = flashing red
boolean color_change = false; // keeps track of change so we don't constantly pump data out to led

// Now we pull in some required libraries

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define NUMPIXELS 1 // only 1 pixel for this application

// Set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Change these values if you'd like to change what the colors look like

#define GREEN  pixels.Color(0,255,0)
#define YELLOW  pixels.Color(255,255,0)
#define RED pixels.Color(255,0,0)
#define BLACK pixels.Color(0,0,0)

// Set up some variables for debouncing the button
// Note that we're assuming a reset pulls it low. Change HIGH to LOW if reset pulls high

int buttonState = HIGH;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();
  pixels.setPixelColor(0, GREEN);
  pixels.show();
  delay(DELAY);

  // set the button for input
  pinMode(RESET_PIN, INPUT_PULLUP); // using internal pull up. Change to simple INPUT if reset pulls HIGH 
  //Serial.begin(115200);
  //Serial.println("I am alive");
  //Serial.print("Yellow time: "); Serial.println(yellow_time);
  //Serial.print("Red time: "); Serial.println(red_time);
  //Serial.print("Flash time: "); Serial.println(flash_time);
  start_time = millis(); // reset start timer after setup to account for any delay in the code above
}

void loop() {
  // first we check for the button being pushed. borrowed most of this straight from the debounce Arduino code
  int reading = digitalRead(RESET_PIN);
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // reset things if we've pushed the button
      // Note that we're assuming a reset pulls low. Change to HIGH if opposite
      if (buttonState == LOW) {
        color_mode = 0;
        color_change = true;
        start_time = millis(); // reset time since last button push
        //Serial.println("Reset!");
      }
    }
  }
  lastButtonState = reading;

  // check to see if we've crossed a threshold
  // first get the current time
  
  current_time = millis();


  // check for crossing threshold time. note that we check both time *and* mode so that we don't keep sending data out to the 
  // led. otherwise, every time we checked for a threshold, it would return true and we'd needlessly update the color

  // check to see if we've exceeded time to change to yellow and we're in green mode

  if ((current_time - start_time) > yellow_time && (color_mode == 0)) {
    color_mode = 1;
    color_change = true;
    //Serial.print("Time expired is: "); Serial.println(current_time - start_time);
    //Serial.println("Changing to yellow!");
    
  }

  // check to see if we've exceeded time to change to red and we're in yellow mode

  if ((current_time - start_time) > red_time && (color_mode == 1)) {
    color_mode = 2;
    color_change = true;
    //Serial.print("Time expired is: "); Serial.println(current_time - start_time);    
    //Serial.println("Changing to red!");
  }

  // check to see if we've exceeded time to start flashing and are in red mode

  if ((current_time - start_time) > flash_time && (color_mode == 2)) {
    color_mode = 3;
    //Serial.print("Time expired is: "); Serial.println(current_time - start_time);   
    //Serial.println("Changing to flashing!");
  }

  switch (color_mode) {
    case 0: // color is green
      if (color_change == true) {
        pixels.setPixelColor(0,GREEN);
        pixels.show();
        delay(DELAY);
        color_change = false; // set to false so we don't keep pumping data out
      }
      break;
    case 1: // color is yellow
      if (color_change == true) {
        pixels.setPixelColor(0,YELLOW);
        pixels.show();
        delay(DELAY);
        color_change = false;
      }
      break;
    case 2: // color is red
      if (color_change == true) {
        pixels.setPixelColor(0,RED);
        pixels.show();
        delay(DELAY);
        color_change = false;
      }
      break;
    case 3: // color is flashing red
      if ((flash_flag == false) && (millis() - red_flash_time) > FLASH_ON) {
        pixels.setPixelColor(0, RED);
        pixels.show();
        delay(DELAY);
        flash_flag = true;
        red_flash_time = millis();
      } // this is somewhat repetitive. I could probably refactor it into something better
      if ((flash_flag == true) && (millis() - red_flash_time) > FLASH_OFF) {
        pixels.setPixelColor(0, BLACK);
        pixels.show();
        delay(DELAY);
        flash_flag = false;
        red_flash_time = millis();        
      }
      break;
  }
}
