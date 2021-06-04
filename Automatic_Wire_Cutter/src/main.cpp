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

/******************* UI details */
#define BUTTON_X 100
#define BUTTON_Y 200
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

//Define button states


boolean RED_state = 0;
boolean GRN_state = 0;
boolean BLU_state = 0;

// Define button array object
Adafruit_GFX_Button buttons[3];

// Define arrays with button text and colors
char buttonlabels[3][5] = {"R", "G", "B"};
uint16_t buttoncolors[6] = {RED, GREEN, BLUE};

// Define object for TFT (LCD)display
MCUFRIEND_kbv tft;

// Define object for touchscreen
// Last parameter is X-Y resistance, measure or use 300 if unsure
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


void setup(void) {

  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));



  tft.reset();

  // Setup the Display
  uint16_t identifier = 0x9486;
  tft.begin(identifier);
  Serial.print("TFT size is "); Serial.print(tft.height()); Serial.print("x"); Serial.println(tft.width());
  tft.setRotation(3);
  tft.fillScreen(BLACK);

  // Draw buttons
  for (uint8_t col = 0; col < 3; col++) {

      int x_coord = BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_Y);
      int y_coord = BUTTON_Y;
      buttons[col].initButton(&tft, 
                                        x_coord,
                                        y_coord, // x, y, w, h, outline, fill, text
                                        BUTTON_W, 
                                        BUTTON_H, 
                                        WHITE, 
                                        buttoncolors[col], 
                                        WHITE,
                                        buttonlabels[col], 
                                        BUTTON_TEXTSIZE);
      buttons[col].drawButton();
      Serial.print("XCoord: ");
      Serial.println(x_coord);
      Serial.print("YCoord: ");
      Serial.println(y_coord);

    
  }

}

void loop(void) {

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

    Serial.print("p.x= ");
    Serial.println(p.x);
    Serial.println("");
    
    Serial.print("p.y= ");
    Serial.println(p.y);
    Serial.println("");

    p.x = p.x + p.y;
    p.y = p.x - p.y;
    p.x = p.x - p.y;

    Serial.print("Calcp.x= ");
    Serial.println(p.x);
    Serial.println("");

    Serial.print("Calcp.y= ");
    Serial.println(p.y);
    Serial.println("");


    p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(),0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(),0);

    Serial.print("Final p.x= ");
    Serial.println(p.x);
    Serial.println("");
    
    Serial.print("Final p.y= ");
    Serial.println(p.y);
    Serial.println("");

  }

  // Go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < 3; b++) {
    if ((buttons[b].contains(p.y, p.x)) && p.x > 10)
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


      delay(200); // UI debouncing
    }
  }

}

