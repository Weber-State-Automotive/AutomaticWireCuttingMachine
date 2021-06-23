/**================================================================================================
 * ?                                           ABOUT
 * @author         :  Scott Hadzik
 * @email          :  scotthadzik@weber.edu
 * @repo           :  https://github.com/Weber-State-Automotive/AutomaticWireCuttingMachine
 * @createdOn      :  March 2019
 * @description    :  Wire Cutting and Stripping Machine. Ada 
 * 
 * 
 * 
 * 
 * 
 * @credit         :  DJ Harrigan Element 14 Presents
 *                        https://www.element14.com/community/docs/DOC-91289/l/episode-368-arduino-automatic-wire-cutter-and-stripper?CMP=SOM-YOUTUBE-E14PRESENTS-EP368-DESC
 *                    Adafruit phone screen demo 
 *                        https://learn.adafruit.com/adafruit-2-8-tft-touch-shield-v2/resistive-touchscreen-paint-demo
 *                    DroneBot Workshop 
 *                        https://dronebotworkshop.com
 *                    Roderek Soest wrote most of the first version of this program
 *                        https://github.com/roderek/E14-wire-cutter-for-Weber-Automotive
 *================================================================================================**/


#include <Arduino.h>
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

// Define object for TFT (LCD)display
MCUFRIEND_kbv tft;

// Define object for touchscreen
// Last parameter is X-Y resistance, measure or use 300 if unsure
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define PIN_SENSOR A8 // Hall effect sensor for determining position of cutter (RED wire to 5V, BLACK wire to GND, BLUE wire to A8)

/**========================================================================
 **                           Adafruit Button variables
 *========================================================================**/
Adafruit_GFX_Button unit_buttons[6];
Adafruit_GFX_Button number_buttons[3];
Adafruit_GFX_Button menu_buttons[3];

/**========================================================================
 **                           Cutting Stepper variables
 *========================================================================**/
#define CUT_DIR_PIN 35
#define CUT_STP_PIN 43
AccelStepper CUT_stepper(1, CUT_STP_PIN, CUT_DIR_PIN);

long current_time = 0;
long last_time = 0;
int delta_time = 100;
long retracted_cut_position = 1700; 
boolean cut_stepper_is_homed = false;

/**========================================================================
 **                           Text field variables
 *========================================================================**/
#define TEXT_X 20
#define TEXT_Y 20
#define TEXT_TSIZE 6
int wire_length = 0;
int wire_qty = 1;
int wire_strip_length = 10;
int cut_values [3] = {wire_length, wire_qty, wire_strip_length};

/**========================================================================
 **                           Menu field variables
 *========================================================================**/

int current_menu_state = 0;

/**========================================================================
 **                           Feed stepper variables
 *========================================================================**/
#define FEED_PIN_ENABLE 25
#define FEED_DIR_PIN 29
#define FEED_STP_PIN 27
AccelStepper FEED_stepper(1, FEED_STP_PIN, FEED_DIR_PIN);
float feed_current_pos = 0;


/**========================================================================
 **                           setButtonState
 *?  Inverts the button to show which one is selected
 *@param activeButton --> Invert this button  
 *@param inactiveButton1 --> draw normal button state  
 *@param inactiveButton2 --> draw normal button state
 *@return void
 *========================================================================**/

void setButtonState(int activeButton, int inactiveButton1, int inactiveButton2){
  menu_buttons[activeButton].drawButton(true);
  menu_buttons[inactiveButton1].drawButton();
  menu_buttons[inactiveButton2].drawButton();
  current_menu_state = activeButton;
}

 /**================================================================================================
  **                           setTextValue
  *?  Sets the value of the text field based on the place
  *@param value int the value to set  
  *@param place int the place in the array 
                          0 -> length
                          1 -> qty
                          2 -> strip   
  *@return void
  *===============================================================================================**/
void setTextValue(int value, int place){
  
    tft.setCursor(TEXT_X + 2 + (place*(160)), TEXT_Y+10);
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(TEXT_TSIZE);
    if (value < 10){
      tft.print("  ");
    }
    else if (value < 100){
      tft.print(" ");
    }
    tft.print(value);
}

/**================================================================================================
 **                                   setupTouchscreen
 *?  Draws the buttons and textfields for the touchscreen 
 *@return void
 *================================================================================================**/

void setupTouchscreen(){
  Serial.println("Setting up Touchscreen...");
  
  /**========================================================================
   **                           Setup the Touchscreen Display
   *========================================================================**/
  tft.reset();
  uint16_t identifier = 0x9486;
  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  //Define button states
  

  /**========================================================================
   **                           Setup the text field
   *========================================================================**/
  setTextValue(wire_length,0);
  setTextValue(wire_qty,1);
  setTextValue(wire_strip_length,2);
  
  /**========================================================================
   **                           Create Unit buttons 
   *========================================================================**/
  
  #define BUTTON_X 40
  #define BUTTON_Y 180
  #define BUTTON_W 60
  #define BUTTON_H 45
  #define BUTTON_SPACING_X 60
  #define BUTTON_Padding 20
  #define BUTTON_SPACING_Y 30
  #define BUTTON_TEXTSIZE 3
  
  const char* buttonlabels[3][2] ={ 
      { "-1", "+1"}, 
      { "-10", "+10"}, 
      { "-50", "+50"} };
  uint16_t buttoncolors[6] = {RED, GREEN, BLUE, YELLOW, GREY, CYAN};

  int function_button = 0;
  for (uint8_t row = 0; row < 3; row++){
    int y_coord = BUTTON_Y + row * 50;
    for (uint8_t col = 0; col < 2; col++) {

        int x_coord = BUTTON_X + col * (80);
        unit_buttons[function_button].initButton(&tft, 
                                x_coord,
                                y_coord, // x, y, w, h, outline, fill, text
                                BUTTON_W, 
                                BUTTON_H, 
                                WHITE, 
                                buttoncolors[col],
                                WHITE,
                                buttonlabels[row][col], 
                                BUTTON_TEXTSIZE);
        unit_buttons[function_button].drawButton();
        function_button++;
    }
    
  }

  /**========================================================================
   **                           Create Menu Buttons
   *========================================================================**/
  #define TITLE_X 80
  #define TITLE_Y 120
  int TITLE_W = (BUTTON_W * 2) + BUTTON_Padding;
  int TITLE_H = BUTTON_H;
  int TITLE_SPACING_X = TITLE_W + BUTTON_Padding;
  int TITLE_Padding = BUTTON_Padding;
  int TITLE_TEXTSIZE = 3;
  const char* menu_labels[3] = { "Length", "Qty", "Strip"};
  uint16_t title_colors[3] = {RED, GREY, BLUE};


  for (uint8_t col = 0; col < 3; col++) {

        int x_coord = TITLE_X + col * (TITLE_W + TITLE_Padding);
        menu_buttons[col].initButton(&tft, 
                                x_coord,
                                TITLE_Y, // x, y, w, h, outline, fill, text
                                TITLE_W, 
                                TITLE_H, 
                                WHITE, 
                                title_colors[col],
                                WHITE,
                                menu_labels[col], 
                                BUTTON_TEXTSIZE);
        menu_buttons[col].drawButton();
  }
/* ---------------------- Make length the active button --------------------- */
  setButtonState(0,1,2);
  Serial.println("Touchscreen Setup");
}
/*============================ END OF Touchscreen Setup ============================*/



/**========================================================================
 **                           Setup Cutting Stepper Motor
 *========================================================================**/
void setupCutStepper(){
  Serial.println("Setting up Stepper Cut...");
  // Setup the cutting stepper motor
  CUT_stepper.setPinsInverted(true, true);
  CUT_stepper.setMaxSpeed(20000);
  CUT_stepper.setAcceleration(40000);
  Serial.println("Cut Stepper Setup");
}

/**========================================================================
 **                           Setup Feed Stepper Motor
 *========================================================================**/
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

/**========================================================================
 **                          Menu Selection
 *========================================================================**/

int setMenuSelection(uint8_t b){
  if (b == 0) { // length   
    setButtonState(0,1,2);
    return 0;
  }
  if (b == 1) { // qty
    setButtonState(1,0,2);
    return 1;
  }
  if (b == 2) { //strip
    setButtonState(2,0,1);
    return 2;
  }
}
/**========================================================================
 **                           Set which multiple to use
 *========================================================================**/
int setMultiple(uint8_t function_button){
    // Serial.println(function_button);
    switch (function_button){
      case 0:
        return -1;
      case 1:
        return 1;
      case 2:
        return -10;
      case 3:
        return 10;
      case 4:
        return -50;
      case 5:
        return 50;
    }
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
  
  // Go thru all the menu buttons, checking if they were pressed
  for (uint8_t b = 0; b < 3; b++) {
    if ((menu_buttons[b].contains(p.y, p.x)) && p.x > 10){
      menu_buttons[b].press(true);  // tell the button it is pressed
      setMenuSelection(b);
    }
  }

  // Go thru all the unit buttons, checking if they were pressed
  
  for (uint8_t function_button = 0; function_button < 6; function_button++) {
      Adafruit_GFX_Button btn = unit_buttons[function_button];
      if ((btn.contains(p.y, p.x)) && p.x > 10){
        btn.press(true);  // tell the button it is pressed
        int change_number_by = setMultiple(function_button);
        Serial.println(change_number_by);
        Serial.println("cut value");
        Serial.println(cut_values[current_menu_state]);
        int current_value = cut_values[current_menu_state];
        int newValue = current_value += change_number_by;
        cut_values[current_menu_state] = newValue;
        Serial.println("new value = ");
        Serial.println(newValue);
        
        setTextValue(current_menu_state, cut_values[current_menu_state]);
        
      }
      delay(20);
  }
  
 
   FEED_stepper.run();

}



