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
#define PIN_ENABLE_FEED 23 // this is the enable for the feed stepper motor, make sure is installed, labeled as th EN slot on the driver, and has a brown wire in our case with our wiring harness. 
#define PIN_SENSOR A8 // Hall effect sensor for determining position of cutter (RED wire to 5V, BLACK wire to GND, BLUE wire to A8)
#define WIRE_QUANT_MIN 1
#define WIRE_QUANT_MAX 100
#define WIRE_MIN_LEN 5 
#define WIRE_AWG 18

// (DRIVER, STEP, DIR)
// "1" after stepCut indicates the use of a motor driver module
// pins to the arduino are as follows
// -- 5V to 5V
// -- step to arduino pin 43
// -- dir to arduino pin 35, 
// -- current resistor and is set to OPEN. 
// stepper motor to the driver for the cutter on the larger motor driver
// -- red to a, blue to a-
// -- green to b, black to b-,  
AccelStepper stepCut(1, 43, 35); 

// (DRIVER Type, step, dir) pinouts are as follows
// -- DIR (Blue) to pin 27 on arduino
// -- STEP (grey) to arduino pin 25
// -- EN aka engage (brown) to arduino pin 23
// -- COMM (yellow) to 5v ardiono pin 21
// -- GND (black) to 24V GND
// -- V+ (red) to 24V+. 
// -- 3 wires are unused: TXD(green), RXD(black), CHOP (white).
// Stepper motor to the driver for the feeder
// -- blue to b2, red to b1
// -- green to a2, black to a1     https://www.omc-stepperonline.com/nema-17-bipolar-59ncm-84oz-in-2a-42x48mm-4-wires-w-1m-cable-and-connector.html?search=17hs19-2004s1//
AccelStepper stepFeed(1, 25, 27);  

boolean ledState = 0;
long curTime = 0;
long lastTime = 0;
int deltaTime = 100;
long retractPos = 1700; //how much should i be open, lower the number, smaller the hole, and make sure the blades are always touching. 
long stripPos = 270; //how shut must i be to strip , smaller the number, the smaller the hole and the deeper the cut
long stripFeedDistance = 0;
long lengthFeedDistance = 0;
long cutPos = -200; //how far do I need to go to make sure I cut it. dont go to far or might damage blades
long targetPos = 0;
boolean isHomed = false;
int bladeCycleState = 0;
int sensorVal = 0;

uint16_t wireQuantity = 0;
uint16_t wireLength = 0; // in milimeters
uint16_t wireStripLength = 0; 
uint16_t wiresCut = 0;
float conductor_diam = 0.64516; // 22 AWG
float feed_diam = 22; // uncalibrated!
float feed_circum = PI * feed_diam; // 69.115mm per rev
float feed_res = feed_circum / 200.0; // .346mm per step


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
          if (sensorVal > 60){  //change me to adjust the home 
            targetPos -= 20; //this is how fast and accurate i find home (which is a closed cutter with no gaps)
            stepCut.moveTo(targetPos);
          } else {
            isHomed = true;
            stepCut.setCurrentPosition(0);
          } 
        }
        stepCut.run();
      }
    break;
    case 'R': // retract
      stepCut.moveTo(retractPos);
    break;
    case 'S': // strip
      stepCut.moveTo(stripPos);
    break;
    case 'C': // cut
      stepCut.moveTo(cutPos);
    break;
  }
}

void setFeedPosition(float position){
  stepFeed.setCurrentPosition(0);
  stripFeedDistance = -(32* round(float(wireStripLength)/feed_res)); // the motor spins counterclockwise, hence the negative on the thirty two
  lengthFeedDistance = -(32* ((wireLength - 2*(wireStripLength))/feed_res));// the motor spins counterclockwise, hence the negative on the thirty two 
  stepFeed.setCurrentPosition(stripFeedDistance); 
  Serial.println('feed on');   
}

void setup(void) {

  setFeedPosition(0);

  Serial.begin(9600);
  tft.reset();

  // Setup the Display
  uint16_t identifier = 0x9486;
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(BLACK);

  // Setup the cutting stepper motor
  stepCut.setPinsInverted(true, true);
  stepCut.setMaxSpeed(20000);
  stepCut.setAcceleration(40000);

  // Setup the feeding stepper motor
  stepFeed.setPinsInverted(true, true);
  stepFeed.setMaxSpeed(2000);
  stepFeed.setAcceleration(6000);
  pinMode(PIN_ENABLE_FEED, OUTPUT);

  digitalWrite(PIN_ENABLE_FEED, LOW);
  delay(200);
  digitalWrite(PIN_ENABLE_FEED, HIGH);
  delay(200);
  digitalWrite(PIN_ENABLE_FEED, LOW);
  setBlade('H');
  setBlade('R');

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
      buttons[b].drawButton();  // draw normal
    }

    if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert!

      delay(200); // UI debouncing
    }
  }

}



