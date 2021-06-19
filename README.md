# AutomaticWireCuttingMachine

# Stepper Motors

- Stepper Feed
    - (DRIVER Type, step, dir) pinouts are as follows
    - -- DIR (Blue) to pin 29 on arduino
    - -- STEP (grey) to arduino pin 27 wht at connector
    - -- EN aka engage (brown) to arduino pin 25 blk at connector
    - -- COMM (yellow) to 5v ardiono pin 23 wht and blk stripe
    - -- GND (black) to 24V GND
    - -- V+ (red) to 24V+. 
    - -- 3 wires are unused: TXD(green), RXD(black), CHOP (white).
    - Stepper motor to the driver for the feeder
    - -- blue to b2, red to b1
    - -- green to a2, black to a1     https://www.omc-stepperonline.com/nema-17-bipolar-59ncm-84oz-in-2a-42x48mm-4-wires-w-1m-cable-and-connector.html?search=17hs19-2004s1//
// Feed Motor Controller variable. 
- Stepper Cut
    - / (DRIVER, STEP, DIR)
    - "1" after stepCut indicates the use of a motor driver module
    - pins to the arduino are as follows
    - -- 5V to 5V
    - -- step to arduino pin 43
    - -- dir to arduino pin 35, 
    - -- current resistor and is set to OPEN. 
    - stepper motor to the driver for the cutter on the larger motor driver
    - -- red to a, blue to a-
    - -- green to b, black to b-,  
    