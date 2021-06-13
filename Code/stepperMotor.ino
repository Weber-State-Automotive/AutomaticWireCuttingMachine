#include <AccelStepper.h>
// Define step constants
#define FULLSTEP 4
 
// Define Motor Pins (2 Motors used)
 
#define motorPin1  8     // Blue
#define motorPin2  9     // Pink
#define motorPin7  6     // Yellow - 28BYJ48 pin 3
#define motorPin8  7     // Orange - 28BYJ48 pin 4
 
// Define two motor objects
AccelStepper stepper(FULLSTEP, motorPin5, motorPin7, motorPin6, motorPin8);
 
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
    stepper.moveTo(-stepper1.currentPosition());
  stepper.run();
}