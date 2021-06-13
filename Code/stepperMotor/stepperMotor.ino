#include <AccelStepper.h>
// Define step constants
#define FULLSTEP 4
 
// Define Motor Pins (2 Motors used)
 
#define feed_dir  29    // Blue
#define feed_step  27     // Pink
 
//AccelStepper mystepper(1, pinStep, pinDirection); A stepper motor controlled by a dedicated driver board.
AccelStepper stepper(1, feed_step, feed_dir); 
 
void setup()
{
  // 1 revolution Motor 1 CW
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(50.0);
  stepper.setSpeed(200);
  stepper.moveTo(2048);  
  
  
}
 
void loop()
{
  //Change direction at the limits
  if (stepper.distanceToGo() == 0) 
    stepper.moveTo(-stepper.currentPosition());
  stepper.run();
}
