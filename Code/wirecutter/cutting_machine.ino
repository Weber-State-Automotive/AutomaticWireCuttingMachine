
#include <Adafruit_CharacterOLED.h>
#include <AccelStepper.h>
#include <Bounce2.h>
#include <Encoder.h>

// READ ME READ ME 
// This is made for Weber State University Automotive program, and is different than the original Element14 project. That being said, the wiring is VERY similar, and we will have a seperate file that is specifically
// for the schools specific system for our systems differs from the Element14 version in mechanical operation, but again has extremely similar wiring. There was real effort to follow the instructions as close as possible, and reflect it in our builds. Please file an issue on github if you need assistance, though turn around times may vary.
// 
//For the buttons and encoder info, we built a break out board to handle the repeated need for the 5v and GND bus, as long as the pins match, 
//and requirements are followed for the pins, it should be fine. While looking at the bottom of the encoder with the pins being on the bottom looking at the pcb, the goes as- 
//follows, pin 1 is GND, pin 2 is 5V, pin 3 to pin 4 on arduino, pin 4 to pin 3 on arduino, pin 5 to pin 2 on arduino. We found that the button we have installed next to the encoder-//
//on the panel seems to not have a use at this time.
#define ENCODER_DO_NOT_USE_INTERRUPTS
#define PIN_ENABLE_FEED 23 // this is the enable for the feed stepper motor, make sure is installed, labeled as th EN slot on the driver, and has a brown wire in our case with our wiring harness. 
#define PIN_SENSOR A8 // Hall effect sensor for determining position of cutter (RED wire to 5V, BLACK wire to GND, BLUE wire to A8)
#define WIRE_QUANT_MIN 1
#define WIRE_QUANT_MAX 100
#define WIRE_MIN_LEN 5 
#define WIRE_AWG 18


Adafruit_CharacterOLED lcd(OLED_V2, 6, 7, 8, 9, 10, 11, 12); //from the back of the screen, from left to right, the pins are as follows, pin 1 goes to GND, pin 2 to 5v, pin 3 to pin 12 arduino,- 
//- pin4 to pin 11 arduino, pin 5 to pin 10 arduino, pin 6 to pin 9 arduino, pin 7&8&9&10 unused. pin 11 to pin 8 arduino, pin 12 to pin 7 arduino, pin 13 to pin 6 arduino, pin 14 unused, pin 15 to 5v, pin 16 to GND 


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


AccelStepper CUT_stepper(1, 43, 35); 

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

AccelStepper FEED_stepper(1, 25, 27);  

Encoder encoder(PIN_ENC_A, PIN_ENC_B); 
Bounce buttonOK = Bounce();
Bounce buttonRED = Bounce();


int curSpool = 0; // in future, give each spool a unique id and track total length cut ( we are working on this, but is a later addition to our machine.)
int curSpoolRemaining = 0;

int mainMode = 0;
int subMode = 0;
int lastSubMode = -99;
int scrollPos = 0;
int lastScrollPos = -99;
long encPos = -999;
long lastEncPos = 0;

long retracted_cut_position = 1700; //how much should i be open, lower the number, smaller the hole, and make sure the blades are always touching. 
long stripPos = 270; //how shut must i be to strip , smaller the number, the smaller the hole and the deeper the cut
long stripFeedDistance = 0;
long lengthFeedDistance = 0;
long cutPos = -200; //how far do I need to go to make sure I cut it. dont go to far or might damage blades
long targetPos = 0;
boolean cut_stepper_is_homed = false;
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
//float feed_res = 0.086;

// Z travel is 1/16" (1.5875mm) per revolution
// both motors are 1.8deg per step (200 per revolution)
// the cut motor driver has a noncofigurable default 10x microstepping rate
// so z resolution = 0.00079375mm per "full" step sent
// leaving us with ~1260 steps per mm z travel

boolean ledState = 0;
long current_time = 0;
long last_time = 0;
int delta_time = 100;

void setup(){

  Serial.begin(115200);
  lcd.begin(16, 2);
  lcd.print("AWCS READY!");
  CUT_stepper.setPinsInverted(true, true);
  FEED_stepper.setPinsInverted(true, true);
  FEED_stepper.setMaxSpeed(2000);
  FEED_stepper.setAcceleration(6000);
  CUT_stepper.setMaxSpeed(20000);
  CUT_stepper.setAcceleration(40000);
  pinMode(PIN_ENABLE_FEED, OUTPUT);
  pinMode(PIN_BUTTON_ENC, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RED, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  buttonOK.attach(PIN_BUTTON_ENC);
  buttonRED.attach(PIN_BUTTON_RED);
  buttonOK.interval(5);
  buttonRED.interval(5);
  
  digitalWrite(PIN_ENABLE_FEED, LOW);
  delay(200);
  digitalWrite(PIN_ENABLE_FEED, HIGH);
  delay(200);
  digitalWrite(PIN_ENABLE_FEED, LOW);
  //setZHOME():
  setBlade('H');
  //lcd.clear();
  printMenu();
  printCursor(0);
  subMode = 5;
  setBlade('R');
}

void loop(){
 // be sure to turn to comment below here during normal operation
 /* curTime = millis(); // VERY BASIC Calibration measurement
  if (curTime - lastTime > deltaTime){
    lastTime = curTime;
     getInput();
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("POS:   ");
    lcd.setCursor(4, 0);
    lcd.print(scrollPos);
    lcd.setCursor(0, 1);
    lcd.print("SEN:    ");
    lcd.setCursor(4, 1);
    lcd.print(sensorVal);
    cutPos = 63 * scrollPos;
    stepCut.moveTo(cutPos);
  } 
*/
  //be sure to turn to comment above here suring normal operation
  CUT_stepper.run();
  
  getInput();
  
  switch(mainMode){
    
    case 0: // MAIN MENU
        switch(subMode){
          case 0: // choose quantity
            if( scrollPos != lastScrollPos){
              if (scrollPos > 999)scrollPos = 1;
              if (scrollPos < 1)scrollPos = 999;
              lastScrollPos = scrollPos;
              wireQuantity = scrollPos;
              lcd.setCursor(5, 0);
              lcd.print("   "); // clear
              lcd.setCursor(5, 0);
              lcd.print(wireQuantity);
            }
            if (buttonOK.fell())returnToMenu();
          break;
          case 1: // choose length (resolution of 5mm)
            if( scrollPos != lastScrollPos){
              if (scrollPos > 99)scrollPos = 0;
              if (scrollPos < 0)scrollPos = 99;
              lastScrollPos = scrollPos;
              wireLength = scrollPos * 5;
              lcd.setCursor(13, 0);
              lcd.print("   "); // clear
              lcd.setCursor(13, 0);
              lcd.print(wireLength);
            }
            if (buttonOK.fell())returnToMenu();
          break;
          case 2: // choose strip length (default strip both ends)
            if( scrollPos != lastScrollPos){
              if (scrollPos > 999)scrollPos = 0;
              if (scrollPos < 0)scrollPos = 999;
              lastScrollPos = scrollPos;
              wireStripLength = scrollPos;
              lcd.setCursor(5, 1);
              lcd.print("   "); // clear
              lcd.setCursor(5, 1);
              lcd.print(wireStripLength);
            }
            if (buttonOK.fell())returnToMenu();
          break;
          case 3: // run
              subMode = 5;
              mainMode = 1; // RUN!
              lcd.clear();
              lcd.home();
              lcd.print("NOW CUTTING:");
              printJobStatus();
              setBlade('R');
              FEED_stepper.setCurrentPosition(0);
              stripFeedDistance = -(32* round(float(wireStripLength)/feed_res)); // the motor spins counterclockwise, hence the negative on the thirty two
              lengthFeedDistance = -(32* ((wireLength - 2*(wireStripLength))/feed_res));// the motor spins counterclockwise, hence the negative on the thirty two 
              Serial.print("FEED STEPS: ");Serial.println(stripFeedDistance);
              Serial.print("FEED DIST: ");Serial.println(lengthFeedDistance);
              
          break;
          case 4: // clear
            resetParameters();
            printMenu();
            returnToMenu();
          
          break;
          case 5: // scrolling
            if( scrollPos != lastScrollPos){
              if (scrollPos > 4)scrollPos = 0;
              if (scrollPos < 0)scrollPos = 4;
              lastScrollPos = scrollPos;
              printCursor(scrollPos);
              }
              if (buttonOK.fell()){
                subMode = scrollPos;
                //delay(200);
            }
          break;
        }
    break;
    
    case 1: // RUN
    
      // loop through quantity, allow mid-job canceling
      switch (bladeCycleState){
        case 0: // initial retract, then feed length of strip
          if (CUT_stepper.distanceToGo() == 0){ // allow the blade to retract
            printJobStatus(); // show wire X of N wires
            bladeCycleState++;
            FEED_stepper.setCurrentPosition(0);
            FEED_stepper.moveTo(stripFeedDistance); // feed length of strip
          }
        break;
        case 1: // feed, then strip
          if (FEED_stepper.distanceToGo() == 0){ // allow strip length to fininsh feeding
            bladeCycleState++;
            setBlade('S');
          }
        break;
        case 2: // strip, then retract
          if (CUT_stepper.distanceToGo() == 0){ // allow time to strip
            bladeCycleState++;
            setBlade('R');
          }
        break;
        case 3: // retract, then feed length of wire
          if (CUT_stepper.distanceToGo() == 0){ // allow retraction
            bladeCycleState++;
            FEED_stepper.setCurrentPosition(0);
            FEED_stepper.moveTo(lengthFeedDistance);
          }
        break;
        case 4: // feed, then strip
          if (FEED_stepper.distanceToGo() == 0){ // allow feed movement etc.
            bladeCycleState++;
            setBlade('S');
          }
        break;
        case 5: // strip, then retract
          if (CUT_stepper.distanceToGo() == 0){ // allow feed movement etc.
            bladeCycleState++;
            setBlade('R');
          }
        break;
         case 6: // retract, then feed
          if (CUT_stepper.distanceToGo() == 0){ // allow feed movement etc.
            bladeCycleState++;
            FEED_stepper.setCurrentPosition(0);
            FEED_stepper.moveTo(stripFeedDistance);
          }
        break;
        case 7: // retract, then feed
          if (FEED_stepper.distanceToGo() == 0){ // allow feed movement etc.
            bladeCycleState++;
            setBlade('C');
          }
        break;
        case 8: // cut, then retract and loop if more wires need to run.
          if (CUT_stepper.distanceToGo() == 0){ // allow feed movement etc.
            bladeCycleState = 0;
            wiresCut++;
            setBlade('R');
          }
        break;
      }
 
      // return to main menu when done or cancel the job
      if (buttonRED.fell() || wiresCut >= wireQuantity){
        mainMode = 0;
        resetParameters();
        subMode = 5;
        returnToMenu();
        printMenu();
        printCursor(0);
      }
    break;
    
  }
  // the motors only really move if there is a remaning position to travel,
  // and this needs to be called often enough that nesting it 
  // inside some other function doesn't help much, so we'll just keep the .run() here
  FEED_stepper.run();
  CUT_stepper.run();
  
  

} // END MAIN LOOP

void printJobStatus(){
   lcd.setCursor(0, 1);
   lcd.print("WIRE     OF   ");
   lcd.setCursor(5, 1);
   lcd.print(wiresCut+1);
   lcd.setCursor(11, 1);
   lcd.print(wireQuantity);
}

void setBlade(char bladePos){
  switch (bladePos){
    case 'H': // home
      while (!cut_stepper_is_homed){
        current_time = millis();
      
        if (current_time - last_time > 100){
          last_time = current_time;
          sensorVal = analogRead(PIN_SENSOR);
          if (sensorVal > 60){  //change me to adjust the home 
            targetPos -= 20; //this is how fast and accurate i find home (which is a closed cutter with no gaps)
            CUT_stepper.moveTo(targetPos);
          } else {
            cut_stepper_is_homed = true;
            CUT_stepper.setCurrentPosition(0);
          } 
        }
        CUT_stepper.run();
      }
    break;
    case 'R': // retract
      CUT_stepper.moveTo(retracted_cut_position);
    break;
    case 'S': // strip
      CUT_stepper.moveTo(stripPos);
    break;
    case 'C': // cut
      CUT_stepper.moveTo(cutPos);
    break;
  }
}

void returnToMenu(){
  scrollPos = subMode;
  subMode = 5;
}

void printCursor(int cursorPos){
  //clear old cursor positions
  lcd.setCursor(0, 0);
  lcd.print(" ");
  lcd.setCursor(8, 0);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(8, 1);
  lcd.print(" ");
  lcd.setCursor(12, 1);
  lcd.print(" ");
  switch(cursorPos){
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(">");
    break;
    case 1:
      lcd.setCursor(8, 0);
      lcd.print(">");
    break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print(">");
    break;
    case 3:
      lcd.setCursor(8, 1);
      lcd.print(">");
    break;
    case 4:
      lcd.setCursor(12, 1);
      lcd.print(">");
    break;
  }
}

void printMenu(){
  lcd.setCursor(0, 0);
  lcd.print(" QTY:    WRL:   ");
  lcd.setCursor(0, 1);
  lcd.print(" STL:    RUN CLR");
}

void resetParameters(){
  wireQuantity = 0;
  wireLength = 0;
  wireStripLength = 0;
  wiresCut = 0;
  bladeCycleState = 0;
  stripFeedDistance = 0;
  lengthFeedDistance = 0;
}

void getInput(){
  buttonOK.update();
  buttonRED.update();
  if (buttonOK.fell()){
    ledState = !ledState;
    digitalWrite(PIN_LED, ledState);
  }
  encPos = encoder.read();
  if (abs(encPos - lastEncPos) > 4){ // did the encoder move one detent?
    if (encPos - lastEncPos > 0){ // positive
      scrollPos++;
    } else { // negative
      scrollPos--;
    }
    lastEncPos = encPos;
  }
}

