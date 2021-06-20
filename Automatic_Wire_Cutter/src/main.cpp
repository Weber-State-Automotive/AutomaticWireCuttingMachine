#include <Arduino.h>
/*
  Modified from Adafruit phone screen demo
  DroneBot Workshop 2019
  https://dronebotworkshop.com

  Modified from dronebotworkshop.com for WSU wire cutting maching
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // LCD library
#include <TouchScreen.h> // Touchscreen Library
#include <MCUFRIEND_kbv.h> // Touchscreen Hardware-specific library
#include <AccelStepper.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

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
#define GREY    0x8410

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
boolean length_button_state = 0;
boolean qty_button_state = 0;
boolean strip_button_state = 0;
const char* current_button_state_list[3] = {"length", "qty", "strip"};
String current_menu_state = current_button_state_list[0];

//Define button array object
Adafruit_GFX_Button buttons[2];

// Define object for TFT (LCD)display
MCUFRIEND_kbv tft;

// Define object for touchscreen
// Last parameter is X-Y resistance, measure or use 300 if unsure
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define PIN_SENSOR A8 // Hall effect sensor for determining position of cutter (RED wire to 5V, BLACK wire to GND, BLUE wire to A8)

// ----------Cut Stepper ---------- //
#define CUT_DIR_PIN 35
#define CUT_STP_PIN 43
AccelStepper CUT_stepper(1, CUT_STP_PIN, CUT_DIR_PIN);

long current_time = 0;
long last_time = 0;
int delta_time = 100;
long retracted_cut_position = 1700; 
boolean cut_stepper_is_homed = false;

// ----------Feed Stepper ---------- //
#define FEED_PIN_ENABLE 25
#define FEED_DIR_PIN 29
#define FEED_STP_PIN 27
AccelStepper FEED_stepper(1, FEED_STP_PIN, FEED_DIR_PIN);
float feed_current_pos = 0;

void setupTouchscreen(){
  Serial.println("Setting up Touchscreen...");
  
  // ---------- Setup the Touchscreen Display ---------- //
  tft.reset();
  uint16_t identifier = 0x9486;
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  
  /******************* UI details */
  #define BUTTON_X 40
  #define BUTTON_Y 180
  #define BUTTON_W 60
  #define BUTTON_H 45
  #define BUTTON_SPACING_X 60
  #define BUTTON_Padding 20
  #define BUTTON_SPACING_Y 30
  #define BUTTON_TEXTSIZE 3
  
  // Define arrays with button text and colors
  const char* buttonlabels[3][6] ={ 
      { "-1", "+1", "-1", "+1", "-1", "+1"}, 
      { "-10", "+10", "-10", "+10", "-10", "+10"}, 
      { "-50", "+50", "-50", "+50", "-50", "+50"} };
  uint16_t buttoncolors[6] = {RED, GREEN, BLUE, YELLOW, GREY, CYAN};


  #define TITLE_X 80
  #define TITLE_Y 120
  int TITLE_W = (BUTTON_W * 2) + BUTTON_Padding;
  int TITLE_H = BUTTON_H;
  int TITLE_SPACING_X = TITLE_W + BUTTON_Padding;
  int TITLE_Padding = BUTTON_Padding;
  int TITLE_TEXTSIZE = 3;

  // ----------Text Box ---------- //
  #define TEXT_X 10
  #define TEXT_Y 40
  #define TEXT_W 220
  #define TEXT_H 50
  #define TEXT_TSIZE 3
  #define TEXT_TCOLOR MAGENTA
  #define TEXT_LEN 12
  char textfield[TEXT_LEN+1] = "";
  uint8_t textfield_i=0;

  // Define arrays with button text and colors
  const char* title_labels[3] = { "Length", "Qty", "Strip"};
  uint16_t title_colors[3] = {RED, GREY, BLUE};

  for (uint8_t col = 0; col < 3; col++) {

        int x_coord = TITLE_X + col * (TITLE_W + TITLE_Padding);
        buttons[col].initButton(&tft, 
                                x_coord,
                                TITLE_Y, // x, y, w, h, outline, fill, text
                                TITLE_W, 
                                TITLE_H, 
                                WHITE, 
                                title_colors[col],
                                WHITE,
                                title_labels[col], 
                                BUTTON_TEXTSIZE);
        buttons[col].drawButton();
    }



  // // Draw buttons
  // for (uint8_t row = 0; row < 3; row++){
  //   int y_coord = BUTTON_Y + row * 50;
  //   for (uint8_t col = 0; col < 6; col++) {

  //       int x_coord = BUTTON_X + col * (80);
  //       buttons[col].initButton(&tft, 
  //                               x_coord,
  //                               y_coord, // x, y, w, h, outline, fill, text
  //                               BUTTON_W, 
  //                               BUTTON_H, 
  //                               WHITE, 
  //                               buttoncolors[col],
  //                               WHITE,
  //                               buttonlabels[row][col], 
  //                               BUTTON_TEXTSIZE);
  //       buttons[col].drawButton();
  //   }
    
  // }
  // for (uint8_t col = 0; col < 3; col++) {
  //   tft.setCursor(TEXT_X + 2 + (col*(160)), TEXT_Y+10);
  //   tft.setTextColor(TEXT_TCOLOR, BLACK);
  //   tft.setTextSize(TEXT_TSIZE);
  //   tft.print(menu_titles[col]);
  // }

  Serial.println("Touchscreen Setup");
}

void setupCutStepper(){
  Serial.println("Setting up Stepper Cut...");
  // Setup the cutting stepper motor
  CUT_stepper.setPinsInverted(true, true);
  CUT_stepper.setMaxSpeed(20000);
  CUT_stepper.setAcceleration(40000);
  Serial.println("Cut Stepper Setup");
}

void setupFeedStepper(){
  Serial.println("Setting up Feed Stepper...");
  
  FEED_stepper.setMaxSpeed(50000.0);
  FEED_stepper.setAcceleration(50000.0);
  FEED_stepper.setCurrentPosition(0);
  FEED_stepper.setPinsInverted(true,true,true);
  // FEED_stepper.moveTo(2048);
  FEED_stepper.setEnablePin(FEED_PIN_ENABLE);
  FEED_stepper.enableOutputs();
  Serial.println("Feed Stepper Setup");
}

void showmsgXY(int x, int y, int sz, const GFXfont *f, const char *msg)
{
    int16_t x1, y1;
    uint16_t wid, ht;
    tft.drawFastHLine(0, y, tft.width(), WHITE);
    tft.setFont(f);
    tft.setCursor(x, y);
    tft.setTextColor(GREEN);
    tft.setTextSize(sz);
    tft.print(msg);
    delay(1000);
}

void setup(void) {

  Serial.begin(9600);
  
  setupTouchscreen();
  setupCutStepper();
  setupFeedStepper();

}

void loop() {

  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  // There is a minimum pressure that is consider valid
  // Pressure of 0 means no pressing
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    p.x = p.x + p.y;
    p.y = p.x - p.y;
    p.x = p.x - p.y;
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.height(),0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(),0);

  }

    // Go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < 3; b++) {
    if ((buttons[b].contains(p.y, p.x)) && p.x > 10)
    {
      Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed
      // current_menu_state = current_button_state_list[b];
      // Serial.println(current_menu_state);
      //Button has been pressed
      if (b == 0) {
        // Toggle Length button on turn off qty and strip   
        length_button_state = !length_button_state;
        // qty_button_state = 0;
        // strip_button_state = 0;
        // buttons[1].drawButton();
        // buttons[2].drawButton();
      }
      if (b == 1) {
        // Toggle qty button on turn off length and strip
        
        length_button_state = !length_button_state;
      }
      if (b == 2) {
        // Toggle Blue status
        strip_button_state = !strip_button_state;
      }
 
      // Button Display
      if (length_button_state == 1) {
        Serial.println("Length ON ");
        FEED_stepper.move(-2000);
      }
      if (qty_button_state == 1) {
        Serial.println("QTY ON");
        FEED_stepper.move(-4000);
      }
      if (strip_button_state == 1) {
        Serial.println("Strip ON");
        FEED_stepper.move(-8000);   
      } 
 
    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }
  }
 
  // now we can ask the buttons if their state has changed
  for (uint8_t b = 0; b < 3; b++) {
    // if (buttons[b].justReleased()) {
    //   Serial.print("Released: "); Serial.println(b);
    //   buttons[b].drawButton();  // draw normal
    // }
 
    if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert!
      delay(100); // UI debouncing
    }
  }
   FEED_stepper.run();

}



