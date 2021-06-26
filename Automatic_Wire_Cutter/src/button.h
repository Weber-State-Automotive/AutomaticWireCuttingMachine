/*
  button.h - Library for initiating adafruit tft button.
  Created by Scott Hadzik June 25,2021.
  Released into the public domain.
*/
#ifndef button_h
#define button_h

#include "Arduino.h"

class Button
{
  public:
    Button(int pin);
    void dot();
    void dash();
  private:
    int _pin;
};

#endif