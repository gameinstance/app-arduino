/*
 * Oscilloscope for Arduino
 * using the Nokia 5110 LCD
 * 
 *  GameInstance.com
 *  2016
 */

#include <SPI.h>
#include <LCDNokia5110.h>

/// the display width in pixels
static const byte WIDTH = 84;
/// the display height in pixels
static const byte HEIGHT = 48;
/// sampling buffer size
static const unsigned int MAX_SIZE = WIDTH;

/// ADC resolution
static const int ADC_RESOLUTION = 1024;
/// ADC half resolution
static const int ADC_RESOLUTION_HALF = 512;
/// the line input
static const int INPUT_LINE = A0;
/// the trigger select button input 
static const int INPUT_BUTTON_TRIGGER = A1;
/// the hold button input 
static const int INPUT_BUTTON_HOLD = A2;
/// the test signal output pin
static const int OUTPUT_TEST_PIN = 9;

/// the state of the automate
byte state, trigger;
/// the display
LCDNokia5110 lcd;
/// values
unsigned int value[MAX_SIZE], currentSample, oldSample, triggerCount;
/// auxiliary values
byte currentValue, oldValue;
/// the buttons state
bool bPress[2], bHold;

/// indicates once that the button was pressed
bool wasPressed(int pin, int index, int threshold = 512) {
  //
  int val = analogRead(pin);
  //Serial.println(val);
  if (val > threshold) {
    // isn't pressed
    if (bPress[index]) {
      // but was before
      bPress[index] = false;
    }
    return false;
  }
  // is pressed
  if (!bPress[index]) {
    // and wasn't before
    bPress[index] = true;
    return true;
  }
  // but was before
  return false;
}

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  lcd.Start();
  lcd.Contrast(45);
  lcd.Light();
  lcd.Fill(false);
  lcd.Text("GameInstance", 8, 8, true);
  lcd.Text(".com", 30, 16, true);
  lcd.Text("DSO v0.1", 16, 32, true);
  lcd.Update();
  oldValue = 255;
  bPress[0] = false;
  bPress[1] = false;
  trigger = 0;
  bHold = 0;
  state = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  if (state == 0) {
    // splash
    delay(1000);
    analogWrite(OUTPUT_TEST_PIN, 127);
    oldSample = ADC_RESOLUTION + 1;
    state = 1;
  }
  
  if (state == 1) {
    // start
    triggerCount = 0;
    if (wasPressed(INPUT_BUTTON_HOLD, 1)) {
      // toggling the trigger mode
      bHold = !bHold;
    }
    if (!bHold) {
      // no hold
      state = 2;
    }
  }
  
  if (state == 2) {
    // triggering
    if (wasPressed(INPUT_BUTTON_TRIGGER, 0)) {
      // toggling the trigger mode
      trigger = (trigger + 1) % 3;
    }
    if (trigger == 0) {
      // no trigger
      state = 3;
    } else if (trigger == 1) {
      // rising edge trigger
      currentSample = analogRead(INPUT_LINE);
      if (oldSample <= ADC_RESOLUTION) {
        // not the first sample
        if ((currentSample >= oldSample) 
          && (currentSample >= ADC_RESOLUTION_HALF) 
          && (oldSample <= ADC_RESOLUTION_HALF)) {
          // event triggered
          state = 3;
        } else {
          // not yet triggered
          triggerCount ++;
          if (triggerCount == MAX_SIZE) {
            // inform user
            lcd.Fill(false);
            lcd.Text("Trigger wait", 8, 8, true);
            lcd.Text("rising edge", 8, 24, true);
            lcd.Update();
          }
        }
      }
      oldSample = currentSample;
    } else if (trigger == 2) {
      // falling edge trigger
      currentSample = analogRead(INPUT_LINE);
      if (oldSample <= ADC_RESOLUTION) {
        // not the first sample
        if ((currentSample <= oldSample) 
          && (currentSample <= ADC_RESOLUTION_HALF) 
          && (oldSample >= ADC_RESOLUTION_HALF)) {
          // event triggered
          state = 3;
        } else {
          // not yet triggered
          triggerCount ++;
          if (triggerCount == MAX_SIZE) {
            // inform user
            lcd.Fill(false);
            lcd.Text("Trigger wait", 8, 8, true);
            lcd.Text("falling edge", 8, 24, true);
            lcd.Update();
          }
        }
      }
      oldSample = currentSample;
    }
  }
  
  if (state == 3) {
    // acquisition
    for (byte i = 0; i < MAX_SIZE; i ++) {
      // 
      value[i] = analogRead(INPUT_LINE);
    }
    state = 4;
  }
  
  if (state == 4) {
    // display
    lcd.Fill(false);
    lcd.Line(0, 0, WIDTH - 1, 0, true);
    lcd.Line(0, 0, 0, HEIGHT - 1, true);
    lcd.Line(WIDTH - 1, HEIGHT - 1, WIDTH - 1, 0, true);
    lcd.Line(WIDTH - 1, HEIGHT - 1, 0, HEIGHT - 1, true);
    for (byte i = 0; i < MAX_SIZE; i ++) {
      // 
      currentValue = (byte)(HEIGHT - (value[i] * (HEIGHT - 2) / ADC_RESOLUTION) - 2);
      if ((oldValue == 255) 
        || (i == 0)
        || (abs(currentValue - oldValue) <= 2)) {
        // 
        lcd.Point(i, currentValue, true); 
      } else {
        // 
        lcd.Line(i - 1, oldValue, i, currentValue, true);
      }
      oldValue = currentValue;
    }
    lcd.Update();
    delay(100);
    state = 1;
  }
}
