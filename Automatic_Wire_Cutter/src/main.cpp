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


void setBlade(char bladePos){
  Serial.println('settingBlade');
  switch (bladePos){
    case 'H': // home
      while (!isHomed){
        curTime = millis();
      
        if (curTime - lastTime > 100){
          lastTime = curTime;
          sensorVal = analogRead(PIN_SENSOR);
          // Serial.println(PIN_SENSOR);
          // if (sensorVal > 60){  //change me to adjust the home 
          //   targetPos -= 20; //this is how fast and accurate i find home (which is a closed cutter with no gaps)
          //   stepCut.moveTo(targetPos);
          // } else {
          //   isHomed = true;
          //   stepCut.setCurrentPosition(0);
          // } 
        }
        CUT_stepper.run();
      }
    break;
    case 'R': // retract
      CUT_stepper.moveTo(retractPos);
    break;
    case 'S': // strip
      CUT_stepper.moveTo(stripPos);
    break;
    case 'C': // cut
      CUT_stepper.moveTo(cutPos);
    break;
  }
}

void setFeedPosition(float position){ 
}

void setupTouchscreen(){
  Serial.println("Setting up Touchscreen...");
  
  // ---------- Setup the Touchscreen Display ---------- //
  tft.reset();
  uint16_t identifier = 0x9486;
  tft.begin(identifier);
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
      Serial.println("Touchscreen Setup");
  }
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
  // Setup the feeding stepper motor
  FEED_stepper.setPinsInverted(true, true, true);
  FEED_stepper.setMaxSpeed(10000.0);
  FEED_stepper.setAcceleration(1000.0);
  FEED_stepper.setCurrentPosition(0);
  FEED_stepper.moveTo(2048);
  FEED_stepper.setEnablePin(FEED_PIN_ENABLE);
  FEED_stepper.enableOutputs();
  Serial.println("Feed Stepper Setup");
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

}



