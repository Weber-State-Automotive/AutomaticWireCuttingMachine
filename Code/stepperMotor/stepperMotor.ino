#include <AccelStepper.h>
// Define step constants
#define FULLSTEP 4
 
// Define Motor Pins (2 Motors used)
 
#define feed_dir  29    // Blue
#define feed_step  27     // Pink
#define PIN_ENABLE_FEED 25
 
//AccelStepper mystepper(1, pinStep, pinDirection); A stepper motor controlled by a dedicated driver board.
AccelStepper stepper(1, feed_dir, feed_step); 
 
void setup()
{
  digitalWrite(PIN_ENABLE_FEED, HIGH);
  // 1 revolution Motor 1 CW
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(50.0);
  stepper.setSpeed(200);
  stepper.setCurrentPosition(0);
  stepper.moveTo(2048);  
  
  
}
 
void loop()
{
  //Change direction at the limits
  if (stepper.distanceToGo() == 0) 
    stepper.moveTo(-stepper.currentPosition());
  stepper.run();
}
