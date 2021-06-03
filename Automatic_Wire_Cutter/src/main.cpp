#include <Arduino.h>

/*
  LCD Touchscreen Demo
  lcd-simple-touch.ino
  Uses touchscreen to display 3 buttons
  Controls 3 LEDs

  Modified from Adafruit phone screen demo
  DroneBot Workshop 2019
  https://dronebotworkshop.com

  Modified from dronebotworkshop.com for WSU wire cutting maching
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // LCD library
#include <TouchScreen.h> // Touchscreen Library
#include <MCUFRIEND_kbv.h> // Touchscreen Hardware-specific library

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Color definitions
#define ILI9341_BLACK       0x0000      /*   0,   0,   0 */
#define ILI9341_NAVY        0x000F      /*   0,   0, 128 */
#define ILI9341_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define ILI9341_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define ILI9341_MAROON      0x7800      /* 128,   0,   0 */
#define ILI9341_PURPLE      0x780F      /* 128,   0, 128 */
#define ILI9341_OLIVE       0x7BE0      /* 128, 128,   0 */
#define ILI9341_LIGHTGREY   0xC618      /* 192, 192, 192 */
#define ILI9341_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define ILI9341_BLUE        0x001F      /*   0,   0, 255 */
#define ILI9341_GREEN       0x07E0      /*   0, 255,   0 */
#define ILI9341_CYAN        0x07FF      /*   0, 255, 255 */
#define ILI9341_RED         0xF800      /* 255,   0,   0 */
#define ILI9341_MAGENTA     0xF81F      /* 255,   0, 255 */
#define ILI9341_YELLOW      0xFFE0      /* 255, 255,   0 */
#define ILI9341_WHITE       0xFFFF      /* 255, 255, 255 */
#define ILI9341_ORANGE      0xFD20      /* 255, 165,   0 */
#define ILI9341_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
#define ILI9341_PINK        0xF81F

/******************* UI details */
#define BUTTON_X 250
#define BUTTON_Y 70
#define BUTTON_W 80
#define BUTTON_H 45
#define BUTTON_SPACING_X 26
#define BUTTON_SPACING_Y 30
#define BUTTON_TEXTSIZE 3

// Define pins for resistive touchscreen
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

// Define touchscreen pressure points
#define MINPRESSURE 10
#define MAXPRESSURE 1000

// Define touchscreen parameters
// Use test sketch to refine if necessary
#define TS_MINX 130
#define TS_MAXX 905

#define TS_MINY 75
#define TS_MAXY 930

#define STATUS_X 10
#define STATUS_Y 65

//Define LED outputs
#define RED_LED 51
#define GRN_LED 49
#define BLU_LED 47

// Define button state variables
boolean RED_state = 0;
boolean GRN_state = 0;
boolean BLU_state = 0;

// Define button array object
Adafruit_GFX_Button buttons[3];

// Define arrays with button text and colors
char buttonlabels[3][5] = {"R", "G", "B"};
uint16_t buttoncolors[6] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE};

//For Show Touch
String msg = ""; // message string to build from keyboard app
long lastTouch = millis();// last touch time for minimum delay between touches
long tThresh = 200;// minimum time between touches
int mode = 0;// current display function - starts with 0 - menu screen
int tMode = 0;// the current touch mode - starts with 0 - menu screen

// Define object for TFT (LCD)display
MCUFRIEND_kbv tft;

// Define object for touchscreen
// Last parameter is X-Y resistance, measure or use 300 if unsure
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

void showTouch() {// diagnostic method to display touch locations on screen

  // with the reset button at the top of the board, the touches
  //are oriented with the x,y origin in the LOWER LEFT CORNER
  if (millis() - lastTouch > tThresh) { // if it's been long enough since last touch
    TSPoint p = ts.getPoint(); // get the screen touch

    if ((p.z > MINPRESSURE && p.z < MAXPRESSURE)) { // if it's a valid touch
      pinMode(YP, OUTPUT);  //restore the TFT control pins
      pinMode(XM, OUTPUT);// for display after detecting a touch
     // tft.fillScreen(BLUE);
      tft.fillRect(70, 80, 100, 30, WHITE);// erase previously displayed coordinates
      tft.setTextSize(2);
      tft.setTextColor(BLACK);
      tft.setCursor(80, 85);// top left corner of text
      tft.print(p.x);
      tft.print(",");
      tft.print(p.y);
      // ***remember TOUCH coordinates are not the same as DISPLAY COORDINATES
      if (abs(p.x - 236) < 70 && abs(p.y - 117 < 20)) {// menu button
        Serial.println("menu press");
      }
    }

    lastTouch = millis();// reset lastTouch for the next touch event
  }
}




void setup(void) {

  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));

  pinMode(RED_LED, OUTPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(BLU_LED, OUTPUT);


  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if (identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if (identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if (identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if (identifier == 0x9341) {
    Serial.println(F("*Found ILI9341 LCD driver"));
  } else if (identifier == 0x7783) {
    Serial.println(F("Found ST7781 LCD driver"));
  } else if (identifier == 0x8230) {
    Serial.println(F("Found UC8230 LCD driver"));
  }
  else if (identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else if (identifier == 0x9481)
  {
    Serial.println(F("Found 0x9481 LCD driver"));
  }
  else if (identifier == 0x9486)
  {
    Serial.println(F("Found 0x9486 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    identifier = 0x9486;
  }

  // Setup the Display
  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());
  tft.setRotation(3);
  tft.fillScreen(BLACK);

  // Draw buttons
  for (uint8_t row = 0; row < 3; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      buttons[col + row * 3].initButton(&tft, BUTTON_Y + col * (BUTTON_W + BUTTON_SPACING_Y),
                                        BUTTON_X + row * (BUTTON_H + BUTTON_SPACING_X), // x, y, w, h, outline, fill, text
                                        BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col + row * 3], ILI9341_WHITE,
                                        buttonlabels[col + row * 3], BUTTON_TEXTSIZE);
      buttons[col + row * 3].drawButton();
    }
  }

}

void loop(void) {

  showTouch();

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);


  // There is a minimum pressure that is consider valid
  // Pressure of 0 means no pressing

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {

    p.x = p.x + p.y;
    p.y = p.x - p.y;
    p.x = p.x - p.y;
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = tft.height() - (map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));

  }

  // Go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < 3; b++) {
    if ((buttons[b].contains(p.x, p.y)) && p.x > 10)
    {
      Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed

      //Button has been pressed
      if (b == 0) {
        // Toggle Red status
        RED_state = !RED_state;
      }
      if (b == 1) {
        // Toggle Green status
        GRN_state = !GRN_state;
      }
      if (b == 2) {
        // Toggle Blue status
        BLU_state = !BLU_state;
      }

      // Button Display
      if (RED_state == 1) {
        digitalWrite(RED_LED, HIGH);
      } else {
        digitalWrite(RED_LED, LOW);
      }
      if (GRN_state == 1) {
        digitalWrite(GRN_LED, HIGH);
      } else {
        digitalWrite(GRN_LED, LOW);
      }
      if (BLU_state == 1) {
        digitalWrite(BLU_LED, HIGH);
      } else {
        digitalWrite(BLU_LED, LOW);
      }

    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // now we can ask the buttons if their state has changed
  for (uint8_t b = 0; b < 3; b++) {
    if (buttons[b].justReleased()) {
      Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
    }

    if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert!


      delay(100); // UI debouncing
    }
  }

}

