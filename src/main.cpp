#include <Arduino.h>
#include <Esplora.h> // Arduino Esplora specific library
#include "TFT_b.h" // Core graphics library
#include <SPI.h> // SPI communications library

#undef DEBUG

// These definitions map display functions to the Adduino Esplora display header pins
#define sclk 15
#define mosi 16
#define cs   7
#define dc   0
#define rst  1

#define MODE_NORMAL 0
#define MODE_ACCELEROMETER 1

#define DIR_STOP 0
#define DIR_DOWN 1
#define DIR_UP 2
#define DIR_RIGHT 3
#define DIR_LEFT 4
#define DIR_DOWN_RIGHT 5
#define DIR_DOWN_LEFT 6
#define DIR_UP_RIGHT 7
#define DIR_UP_LEFT 8

#define JOY_DOWN 0
#define JOY_LEFT 1
#define JOY_UP 2
#define JOY_RIGHT 3

//#define SWITCH_1
//#define SWITCH_2 
#define BTN_SW3 6
//#define SWITCH_4

const byte buttons[] = {
  JOYSTICK_DOWN,
  JOYSTICK_LEFT,
  JOYSTICK_UP,
  JOYSTICK_RIGHT,
  SWITCH_RIGHT, 
  SWITCH_LEFT, 
  SWITCH_UP, 
  SWITCH_DOWN, 
};

#define sound_pin 6 // Direct sound on Esplora (not currently used but you can use with tone() function)
                    // instead of using Esplora.tone


void displayString(byte x, byte y, char *text, uint16_t color) { // write string to LCD
  EsploraTFT.setCursor(x,y);
  EsploraTFT.setTextColor(color);
  EsploraTFT.print(text);
}

void displayChar(byte x, byte y, char text, uint16_t color) {  // write character to LCD
  EsploraTFT.setCursor(x,y);
  EsploraTFT.setTextColor(color);
  EsploraTFT.print(text);
}

// display an unsigned character  on LCD (if you have a signed value, use displayInt2)
void displayInt(unsigned int num, byte nx, byte ny, unsigned int color, unsigned int backcolor) {
  EsploraTFT.fillRect(nx, ny, 29, 7, backcolor);
  displayChar(nx+24, ny, 48+(num%10), color);
  if(num>9) {
    displayChar(nx+18, ny, 48+(num%100)/10, color);
    if(num>99) {
      displayChar(nx+12, ny, 48+(num%1000)/100, color);
      if(num>999) {
        displayChar(nx+6, ny, 48+(num%10000)/1000, color);
        if(num>9999) {
          displayChar(nx, ny, 48+(num%100000)/10000, color);
        }
      }
    }
  }
}

void displayInt2(int num, byte nx, byte ny, unsigned int color, unsigned int backcolor) {
  EsploraTFT.fillRect(nx, ny, 29, 7, backcolor); // clear old value
  EsploraTFT.setCursor(nx,ny);
  EsploraTFT.setTextColor(color);
  EsploraTFT.print(num);  // print handles signs, leading space, etc. in this version
}

void DisplaySplash() {   // display first screen which is also the main menu
  EsploraTFT.fillScreen(ST7735_BLACK);
  EsploraTFT.setTextSize(2);
  displayString(0, 0,"ArduCar",ST7735_GREEN);
  displayString(0,16,"Remote", ST7735_GREEN); 
  EsploraTFT.setTextSize(1); 
  displayString(0,61,"MENU:",ST7735_WHITE);
  displayString(0,78,"- Bt 1: Control manual", ST7735_WHITE);
  displayString(0,91,"- Bt 2: Auto", ST7735_WHITE);
  displayString(0,104,"- Bt 3: GPS", ST7735_WHITE);
}

void ManualControl() {
  boolean buttonStates[8];
  boolean oldButtonStates[8];
  boolean somethingChanged;
  boolean normalMode = true, accelerometerMode = false;
  unsigned int carBearing = DIR_STOP, oldCarBearing = DIR_STOP;
  unsigned int carSpeed, oldCarSpeed = 11;
  int x_axis, y_axis, z_axis;  // accelerometer read values

  EsploraTFT.fillScreen(ST7735_BLACK);
  EsploraTFT.fillRoundRect(0, 0, 160, 16, 4, ST7735_BLUE);
  EsploraTFT.setTextSize(1);
  displayString(8, 4, "ArduCar - Manual Control", ST7735_BLACK);
  EsploraTFT.fillRect(5, 50, 60, 20, 0b0111101110101111);
  EsploraTFT.fillRect(25, 30, 20, 60, 0b0111101110101111);
  EsploraTFT.fillCircle(35, 60, 6, ST7735_BLACK);
  // RIGHT
  EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_BLACK);
  // LEFT
  EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_BLACK);
  // UP
  EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_BLACK);
  // DOWN
  EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_BLACK);

  // Accelerometer setting
  displayString(80, 30, "Accel.:", ST7735_WHITE);
  displayString(125, 30, "OFF", ST7735_WHITE);
  

//  displayString(0,107,"Press down on joystick", ST7735_YELLOW);
//  displayString(0,117," when done",ST7735_YELLOW);

  while(Esplora.readJoystickButton()==HIGH) {  // keep working until joystick clicked down
    // Read all buttons
    somethingChanged = false;
    for (byte thisButton=0; thisButton<8; thisButton++) {
      buttonStates[thisButton] = Esplora.readButton(buttons[thisButton]);
      if (buttonStates[thisButton] != oldButtonStates[thisButton])
        somethingChanged = true;
    }
    if (accelerometerMode || somethingChanged) {
#ifdef DEBUG
      if (somethingChanged) {
        Serial.println("BUTTON CHANGE");
        for (byte thisButton=0; thisButton<8; thisButton++) {
          Serial.print(buttonStates[thisButton]);
          Serial.print(" ");
        }
        Serial.println();
      }
#endif
      // Check mode buttons
      // Check SW3
      if ((buttonStates[BTN_SW3] == 0) && (oldButtonStates[BTN_SW3] != buttonStates[BTN_SW3])) {
        // Change Mode
        normalMode = !normalMode;
        accelerometerMode = !accelerometerMode;
        EsploraTFT.fillRect(125, 30, 30, 10, ST7735_BLACK);
        if (accelerometerMode) {
          displayString(125, 30, "ON", ST7735_GREEN);
        } else {
          displayString(125, 30, "OFF", ST7735_WHITE);
        }
#ifdef DEBUG
        Serial.println("MODE CHANGE");
        Serial.print("NORMAL = ");
        Serial.println(normalMode);
        Serial.print("ACCELEROMETER = ");
        Serial.println(accelerometerMode);
#endif
      }

      // Check direction buttons
      carBearing = 11; // Default new direction set to non valid
      if (normalMode) {
        // Check joystick
        if (buttonStates[JOY_DOWN] == 0)
          // DOWN, KEEP OTHERS
          if (buttonStates[JOY_LEFT] == 0)
            // DOWN-LEFT
            carBearing = DIR_DOWN_LEFT;
          else if (buttonStates[JOY_RIGHT] == 0)
            // DOWN-RIGHT
            carBearing = DIR_DOWN_RIGHT;
          else
            // only DOWN
            carBearing = DIR_DOWN;

        if (buttonStates[JOY_UP] == 0)
          // DOWN, KEEP OTHERS
          if (buttonStates[JOY_LEFT] == 0)
            // DOWN-LEFT
            carBearing = DIR_UP_LEFT;
          else if (buttonStates[JOY_RIGHT] == 0)
            // DOWN-RIGHT
            carBearing = DIR_UP_RIGHT;
          else
            // only DOWN
            carBearing = DIR_UP;

        if ((buttonStates[JOY_DOWN] == 1) && (buttonStates[JOY_UP] == 1))
          if (buttonStates[JOY_LEFT] == 0)
            // DOWN-LEFT
            carBearing = DIR_LEFT;
          else if (buttonStates[JOY_RIGHT] == 0)
            // DOWN-RIGHT
            carBearing = DIR_RIGHT;

      } else {
        // Check accelerometer
        x_axis = Esplora.readAccelerometer(X_AXIS);
        y_axis = Esplora.readAccelerometer(Y_AXIS);
        z_axis = Esplora.readAccelerometer(Z_AXIS);
        // x < 0 is RIGHT, x > 0 is lEFT
        // y < 0 is UP, y > 0 is DOWN
        // Chech direction to apply
        // Check joystick
        if (y_axis > 90)
          // DOWN, KEEP OTHERS
          if (x_axis > 50)
            // DOWN-LEFT
            carBearing = DIR_DOWN_LEFT;
          else if (x_axis < -10)
            // DOWN-RIGHT
            carBearing = DIR_DOWN_RIGHT;
          else
            // only DOWN
            carBearing = DIR_DOWN;

        if (y_axis < 0)
          // UP, KEEP OTHERS
          if (x_axis > 50)
            // UP-LEFT
            carBearing = DIR_UP_LEFT;
          else if (x_axis < -10)
            // UP-RIGHT
            carBearing = DIR_UP_RIGHT;
          else
            // only UP
            carBearing = DIR_UP;

        if ((y_axis < 90) && (y_axis > -10))
          if (x_axis > 50)
            // LEFT
            carBearing = DIR_LEFT;
          else if (x_axis < -10)
            // RIGHT
            carBearing = DIR_RIGHT;
      }

      // Process direction change
      if (carBearing != oldCarBearing) {
        // Redraw directions
        // DOWN
        EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_BLACK);
        // LEFT
        EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_BLACK);
        // UP
        EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_BLACK);
        // RIGHT
        EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_BLACK);
        // STOP
        EsploraTFT.fillCircle(35, 60, 6, ST7735_BLACK);
        switch (carBearing) {
          case DIR_UP:
            EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("UP");
#endif
            break;
          case DIR_UP_RIGHT:
            EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_GREEN);
            EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("UP_RIGHT");
#endif
            break;
          case DIR_UP_LEFT:
            EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_GREEN);
            EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("UP_LEFT");
#endif
            break;
          case DIR_RIGHT:
            EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("RIGHT");
#endif
            break;
          case DIR_LEFT:
            EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("LEFT");
#endif
            break;
          case DIR_DOWN:
            EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("DOWN");
#endif
            break;
          case DIR_DOWN_RIGHT:
            EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_GREEN);
            EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("DOWN_RIGHT");
#endif
            break;
          case DIR_DOWN_LEFT:
            EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_GREEN);
            EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("DOWN_LEFT");
#endif
            break;
          default:
            // STOP
            EsploraTFT.fillCircle(35, 60, 6, ST7735_GREEN);
#ifdef DEBUG
            Serial.println("STOP");
#endif
        }
        oldCarBearing = carBearing;
      }

    }
    // Store new button states
    if (somethingChanged)
      for (byte thisButton=0; thisButton<8; thisButton++) {
        oldButtonStates[thisButton] = buttonStates[thisButton];
      }
    // Check Speed control
    carSpeed = map(Esplora.readSlider(), 1023, 0, 0, 10);
    if(carSpeed != oldCarSpeed) {   // Slider changed
      EsploraTFT.fillRect(10, 100, 140, 20, ST7735_BLACK);
      EsploraTFT.drawRect(9, 99, 142, 22, ST7735_WHITE);
      EsploraTFT.fillRect(10, 100, carSpeed * 14, 20, ST7735_MAGENTA);
      oldCarSpeed = carSpeed;  // save for next time
#ifdef DEBUG
      Serial.print("NEW SPEED: ");
      Serial.println(carSpeed);
#endif
    }
    delay(20); // Anti-bounce delay
  }

/*
   // for controller output, Iterate through all the buttons:
   for (byte thisButton=0; thisButton<8; thisButton++) {
    boolean lastState = buttonStates[thisButton];
    boolean newState = Esplora.readButton(buttons[thisButton]);
    if (lastState != newState) { // Something changed!
      switch(thisButton) {
        case JOYSTICK_DOWN:
          if(newState==1) 
            EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_BLACK);
          else
            EsploraTFT.fillTriangle(35, 87, 42, 73, 28, 73, ST7735_GREEN);
          break;
        case JOYSTICK_LEFT:
          if(newState==1)
            EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_BLACK);
          else
            EsploraTFT.fillTriangle(8, 60, 22, 53, 22, 67, ST7735_GREEN);
          break;
        case JOYSTICK_UP:
          if(newState==1) 
            EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_BLACK);
          else
            EsploraTFT.fillTriangle(35, 33, 28, 47, 42, 47, ST7735_GREEN);
          break;
        case JOYSTICK_RIGHT:
         if(newState==1) 
            EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_BLACK);
          else
            EsploraTFT.fillTriangle(48, 53, 62, 60, 48, 67, ST7735_GREEN);
          break;
        case SWITCH_RIGHT:
          if(newState==1) 
            displayString(104,14,"S4",ST7735_WHITE); 
          else 
            displayString(104,14,"S4",ST7735_GREEN);
          break;
        case SWITCH_LEFT:
          if(newState==1) 
            displayString(68,14,"S2",ST7735_WHITE); 
          else 
            displayString(68,14,"S2",ST7735_GREEN);
          break;
        case SWITCH_UP:
          if(newState==1) 
            displayString(86,14,"S3",ST7735_WHITE); 
          else 
            displayString(86,14,"S3",ST7735_GREEN);
          break;
        case SWITCH_DOWN:
          if(newState==1) 
            displayString(50,14,"S1",ST7735_WHITE); 
          else 
            displayString(50,14,"S1",ST7735_GREEN);
          break;
      } 
    }
    // Store the new button state, so you can sense a difference later:
    buttonStates[thisButton] = newState;
  }

  carSpeed = map(Esplora.readSlider(), 1023, 0, 0, 10);
  if(carSpeed != oldCarSpeed) {   // Slider changed
    EsploraTFT.fillRect(10, 100, 140, 20, ST7735_BLACK);
    EsploraTFT.drawRect(9, 99, 142, 22, ST7735_WHITE);
    EsploraTFT.fillRect(10, 100, carSpeed * 14, 20, ST7735_MAGENTA);
    oldCarSpeed = carSpeed;  // save for next time
  }
  
  x_axis = Esplora.readAccelerometer(X_AXIS);
  y_axis = Esplora.readAccelerometer(Y_AXIS);
  z_axis = Esplora.readAccelerometer(Z_AXIS);
  if(x_axis!=oldx) {
    displayInt2(x_axis,30,96,ST7735_WHITE,ST7735_BLACK);
  }
  if(y_axis!=oldy) {
    displayInt2(y_axis,70,96,ST7735_WHITE,ST7735_BLACK);
  }
  if(z_axis!=oldz) {
    displayInt2(z_axis,110,96,ST7735_WHITE,ST7735_BLACK);
  }
  oldx=x_axis; oldy=y_axis; oldz=z_axis;  // store values for comparison next refresh
  
*/


}
 
void setup() {
  // Put this line at the beginning of every sketch that uses the GLCD
  EsploraTFT.begin();
  EsploraTFT.background(0, 0, 0);
//  EsploraTFT.fillScreen(ST7735_RED);
//  EsploraTFT.fillScreen(ST7735_GREEN);
//  EsploraTFT.fillScreen(ST7735_BLUE);
//  EsploraTFT.fillScreen(ST7735_WHITE);
//  EsploraTFT.background(0, 0, 0);
  EsploraTFT.setTextWrap(true); // Allow text to run off right edge
  EsploraTFT.setCursor(0,0);
  EsploraTFT.setTextColor(ST7735_WHITE);
#ifdef DEBUG  
  EsploraTFT.println("Waiting for serial...");
  Serial.begin(115200);
  while (!Serial) {
    delay(10); //  wait for serial port to connect. Needed for native USB
  }
  Serial.println("START");
#endif
}

void loop() {
  int S1, S2, S3;   // holds values of switches 1 to 3
  DisplaySplash();  // Display splash page and main menu
  S1=HIGH; S2=HIGH; S3=HIGH;
  while(S1==HIGH && S2==HIGH && S3==HIGH) {  // keep reading buttons until one is pressed 
     S1 = Esplora.readButton(SWITCH_1);
     S2 = Esplora.readButton(SWITCH_2);  
     S3 = Esplora.readButton(SWITCH_3);
     if(S1==LOW) 
       ManualControl();  // if Switch 1 is pressed, test outputs
//     else if(S2==LOW) 
//       TestLED();      // if switch 2 is [ressed, do LED mixer
//     else if(S3==LOW)
//       TestSound();    // if switch 3 is pressed, do buzzer test
  }  // end while (breaks out if button presed and routine done (meaning go back to splash page)
}