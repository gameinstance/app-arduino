/*
 * STM32 Digital Oscilloscope
 * using the NT35702 2.4 inch TFT display 
 * https://www.gameinstance.com/post/33/Simple-STM32-Digital-Oscilloscope
 * 
 *  GameInstance.com
 *  2016-2017
 */

#include <Adafruit_ILI9341_8bit_STM.h>
#include <Adafruit_GFX.h>

static const int TIME_BUTTON = PB1;
static const int TRIGGER_BUTTON = PB10;
static const int FREEZE_BUTTON = PB11;
static const int TEST_SIGNAL = PA8;
static const int CHANNEL_1 = PB0;

static const int BLACK   = 0x0000;
static const int BLUE    = 0x001F;
static const int RED     = 0xF800;
static const int GREEN   = 0x07E0;
static const int CYAN    = 0x07FF;
static const int MAGENTA = 0xF81F;
static const int YELLOW  = 0xFFE0;
static const int WHITE   = 0xFFFF;

static const int BACKGROUND_COLOR = BLUE;
static const int DIV_LINE_COLOR = GREEN;
static const int CH1_SIGNAL_COLOR = YELLOW;

static const int ADC_RESOLUTION = 4096; // units
static const int EFFECTIVE_VERTICAL_RESOLUTION = 200; // pixels
static const int SCREEN_HORIZONTAL_RESOLUTION = 320; // pixels
static const int SCREEN_VERTICAL_RESOLUTION = 240; // pixels
static const int DIVISION_SIZE = 40; // pixels
static const float ADC_SCREEN_FACTOR = (float)EFFECTIVE_VERTICAL_RESOLUTION / (float)ADC_RESOLUTION;
static const int BUFFER_SIZE = 512; // bytes
static const byte TRIGGER_THRESOLD = 127; // units

Adafruit_ILI9341_8bit_STM tft;
byte data[BUFFER_SIZE], bk[BUFFER_SIZE];
int i, j, h = 1, h2 = -1, counter, startIndex, xOffset = 10, yOffset = 20, temp, old;
byte state = 0, k, dt = 0, dti = 5, trigger = 1, freeze = 0;
bool bPress[3], bChannelChange = true, bScreenChange = true;
const byte DT_LIST[] = {0, 8, 12, 14, 15, 16, 17, 18, 20, 24, 32};
const int DT_DIV[] = {12, 24, 49, 98, 196, 392, 783, 2, 3, 7, 13};
volatile byte tmpdata;

bool wasPressed(int pin, int index) {
  //
  if (HIGH == digitalRead(pin)) {
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
  // 
  Serial.begin(9600);
  tft.begin();
  tft.setRotation(3);
  bPress[0] = false;
  bPress[1] = false;
  bPress[2] = false;
}

void loop() {
  //
  if (state == 0) {
    // 
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setCursor(15, 100);
    tft.setTextColor(YELLOW);
    tft.setTextSize(3);
    tft.println("GameInstance.com");
    analogWrite(TEST_SIGNAL, 127);
    delay(1500);
    tft.fillScreen(BACKGROUND_COLOR);
    state = 1;
  }
  if (state == 1) {
    // init
    counter = 0;
    startIndex = -1;
    old = -1;
    i = 0;
    state = 2;
  }
  if (state == 2) {
    // buttons
    if (wasPressed(TIME_BUTTON, 0)) {
      // toggling the time division modes
      dti ++;
      dti = dti % sizeof(DT_LIST);
      dt = DT_LIST[dti];
      h = (dti < 5) ? -2 * (dt - 16) : 1;
      bChannelChange = true;
    }
    if (wasPressed(TRIGGER_BUTTON, 1)) {
      // toggling the trigger mode
      trigger ++;
      trigger = trigger % 3;
      bChannelChange = true;
    }
    if (wasPressed(FREEZE_BUTTON, 2)) {
      // toggling the freeze screen
      freeze ^= 0x01;
      bChannelChange = true;
    }
    if (freeze) {
      // frozen screen
      state = 5;
    } else {
      // live screen
      state = 3;
    }
  }
  if (state == 3) {
    // acquisition
    for (;counter < BUFFER_SIZE;) {
      //
      for (k = 0; k < dt - 16; k ++) {
        // 
        tmpdata = (byte) ((float)analogRead(CHANNEL_1) * ADC_SCREEN_FACTOR) + 1;
      }
      data[i] = (byte) ((float)analogRead(CHANNEL_1) * ADC_SCREEN_FACTOR) + 1;
      if (startIndex < 0) {
        // trigger condition not met
        if (trigger == 1) {
          // raising front trigger
          if ((old > -1) && (old < TRIGGER_THRESOLD) && (data[i] >= TRIGGER_THRESOLD)) {
            // 
            startIndex = i;
          }        
        } else if (trigger == 2) {
          // descending front trigger
          if ((old > -1) && (old > TRIGGER_THRESOLD) && (data[i] <= TRIGGER_THRESOLD)) {
            // 
            startIndex = i;
          }
        } else {
          // no trigger
          startIndex = i;
        }
      } else {
        // trigger condition met
        counter ++;
      }
      i ++;
      if (i >= BUFFER_SIZE) i = 0;
      old = data[i];
    }
    state = 4;
  }
  if (state == 4) {
    // 
    if (bScreenChange) {
      // 
      bScreenChange = false;
      tft.fillScreen(BACKGROUND_COLOR);
      bChannelChange = true;
    } else {
      // clear previous wave
      if (h2 > 0) {
        // 
        for (i = 0, j = 0; j < SCREEN_HORIZONTAL_RESOLUTION; i ++, j += h2) {
          // 
          tft.drawLine(
            j, 
            bk[i], 
            j + h2, 
            bk[i + 1], 
            BACKGROUND_COLOR);
        }
      }
    }
    // re-draw the divisions
    for (i = 0; i < SCREEN_HORIZONTAL_RESOLUTION; i += DIVISION_SIZE) {
      // 
      for (j = 0; j < SCREEN_VERTICAL_RESOLUTION; j += ((i == 160) ? 8 : DIVISION_SIZE)) {
        // 
        tft.drawLine(i - 1, j, i + 1, j, DIV_LINE_COLOR);
      }
    }
    for (i = SCREEN_VERTICAL_RESOLUTION; i > 13; i -= DIVISION_SIZE) {
      // 
      for (j = 0; j < SCREEN_HORIZONTAL_RESOLUTION; j += ((i == 120) ? 8 : DIVISION_SIZE)) {
        //
        tft.drawLine(j, i - 1, j, i + 1, DIV_LINE_COLOR);
      }
    }
    // draw current wave
    for (i = 0, j = 0; j < SCREEN_HORIZONTAL_RESOLUTION; i ++, j += h) {
      // 
      tft.drawLine(
        j, 
        SCREEN_VERTICAL_RESOLUTION - (yOffset + data[(i + startIndex + BUFFER_SIZE - xOffset) % BUFFER_SIZE]), 
        j + h, 
        SCREEN_VERTICAL_RESOLUTION - (yOffset + data[(i + 1 + startIndex + BUFFER_SIZE - xOffset) % BUFFER_SIZE]), 
        CH1_SIGNAL_COLOR);
      bk[i] = SCREEN_VERTICAL_RESOLUTION - (yOffset + data[(i + startIndex + BUFFER_SIZE - xOffset) % BUFFER_SIZE]);
    }
    h2 = h;
    state = 5;
  }
  if (state == 5) {
    // 
    if (bChannelChange) {
      // channel change
      bChannelChange = false;
      tft.fillRect(0, 0, SCREEN_HORIZONTAL_RESOLUTION, 12, CH1_SIGNAL_COLOR);
      tft.setCursor(10, 3);
      tft.setTextColor(BLUE);
      tft.setTextSize(1);
      String s = "CH1 ";
      s += 660;
      s += "mV ";
      s += DT_DIV[dti];
      s += ((dti < 7) ? "us" : "ms");
      tft.print(s);
      if (trigger == 1) {
        // raising front trigger
        tft.drawLine(110, 10, 113, 10, BLACK);
        tft.drawLine(113, 3, 113, 10, BLACK);
        tft.drawLine(113, 3, 116, 3, BLACK);
      } else if (trigger == 2) {
        // descending front trigger
        tft.drawLine(110, 3, 113, 3, BLACK);
        tft.drawLine(113, 3, 113, 10, BLACK);
        tft.drawLine(113, 10, 116, 10, BLACK);
      } else {
        // no trigger
        tft.drawLine(110, 3, 116, 10, BLACK);
        tft.drawLine(110, 10, 116, 3, BLACK);
      }
      if (freeze) {
        // 
        tft.setCursor(145, 3);
        tft.setTextColor(RED);
        tft.setTextSize(1);
        tft.print("Freeze");
      }
      tft.setCursor(210, 3);
      tft.setTextColor(BLACK);
      tft.setTextSize(1);
      tft.print("GameInstance.com");
    }
  }
  delay(100);
  state = 1;
}
