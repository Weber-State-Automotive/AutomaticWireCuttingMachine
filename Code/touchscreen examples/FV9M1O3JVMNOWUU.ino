/* tft Demo for Instructables
    May 2019 - Gord Payne
    Uses MCUfriend.com 2.4" TFT display shield for Uno
*/
#define MINPRESSURE 10 // set pressure thresholds for touchscreen
#define MAXPRESSURE 1000
#include <Adafruit_GFX.h> // graphics library
#include <MCUFRIEND_kbv.h> // screen control library
MCUFRIEND_kbv tft; // define tft object

// configure the Analog ports
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin


// MCUFRIEND UNO shield shares pins with the TFT.
#define YP A1   //A3 for ILI9320
#define YM 7    //9
#define XM A2
#define XP 6    //8 

#include <TouchScreen.h>
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); // define touch object
TSPoint tp; //Touchscreen_due branch uses Point
// Define some colours for use in screens
#define  BLACK   0x0000
#define  BLUE    0x001F
#define  GREEN   0x07E0
#define  YELLOW  0xFFE0
#define  WHITE   0xFFFF
#define  CHERRY   0xF020
// for help defining colours, go to: http://www.barth-dev.de/online/rgb565-color-picker/
/* click on the colour square and pick your colour. Then lookunder the RGB565 heading
    and you'll get the hex byte representation for that colour
*/

String cString = "qwerty";// characters for keyboard

char  letter[] = {'q', 'w', 'e', 'r', 't', 'y'};// letters for the keyboard app
int letX[] =     { 194, 291, 402, 516, 629, 739}; // the x and y coordinates for the letters on the keyboard
int letY[] =     { 316, 316, 316, 316, 316, 316};// QWERTY only provided

int funcX[] = {527, 527}; // x and y coordinates for two MENU buttons
int funcY[] = {632, 393};
int func[] = {1, 2}; // '1' is Saucy '7' Ball, '2' is the keyboard app

String msg = ""; // message string to build from keyboard app
long lastTouch = millis();// last touch time for minimum delay between touches
long tThresh = 200;// minimum time between touches
int mode = 0;// current display function - starts with 0 - menu screen
int tMode = 0;// the current touch mode - starts with 0 - menu screen

void setup()
{
  Serial.begin(9600);
  digitalWrite(A0, HIGH);
  pinMode(A0, OUTPUT);
  uint16_t ID = tft.readID(); // get the screen's ID
  tft.begin(ID);// assign the ID to the screen object
  tft.setRotation(0);// LANDSCAPE MODE = 1, Portrait mode = 0
  randomSeed(analogRead(A5));// use this for the Saucy '7' Bball app
  splash(); // display the opening screen for your sketch

}

void loop() {
  showTouch();// method to display touch coordinates on screen
    

}

///////////////////////////

void splash() {// Startup splash screen
  // DISPLAY x and y coordinates are measured from the top left of the screen in PORTRAIT MODE
  tft.fillScreen(WHITE);
  tft.setTextSize(5);
  tft.setCursor(80, 40);
  tft.setTextColor(BLUE);
  tft.print("TFT");
  tft.setTextSize(2);
  tft.setCursor(50, 140);
  tft.setTextColor(CHERRY);
  tft.print("Instructable");
  tft.setTextSize(3);
  tft.setCursor(90, 200);
  tft.setTextColor(BLACK);
  tft.print("Demo");

  delay(2500);// duration for the splash screen to remain displayed
}

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
