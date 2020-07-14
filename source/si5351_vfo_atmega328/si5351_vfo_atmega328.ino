
/**
 * VFO based on SI5351 DDS controlled by Arduino.
 * 
 * This VFO is part of a very small shortwave transmitter.  
 * The idea is to be able to transmit on any frequency in the HF band (3 ˜ 30 MHz). 
 * The main motivation for building this shortwave transmitter is to be able to do experiments and 
 * tests during the development of the Arduino libraries for DSP receivers based on SI473X, Si4844, AKC695X, KT0915 and others.
 * This was necessary due to the location of the author of these libraries does not allow  good reception  of 
 * shortwave broadcast stations most of the time. 
 * 
 * [PU2CLR Si4735 Library for Arduino](https://pu2clr.github.io/SI4735/). This library was built based on “Si47XX PROGRAMMING GUIDE; AN332” and it has support to FM, AM and SSB modes (LW, MW and SW). It also can be used on all members of the SI47XX family respecting, of course, the features available for each IC version;
 * [PU2CLR SI4844 Arduino Library](https://github.com/pu2clr/SI4844). This is an Arduino library for the SI4844, BROADCAST ANALOG TUNING DIGITAL DISPLAY AM/FM/SW RADIO RECEIVER,  IC from Silicon Labs.  It is available on Arduino IDE. This library is intended to provide an easier interface for controlling the SI4844.
 * [PU2CLR AKC695X Arduino Library](https://pu2clr.github.io/AKC695X/). The AKC695X is a family of IC DSP receiver from AKC technology. The AKC6955 and AKC6959sx support AM and FM modes. On AM mode the AKC6955 and AKC6959sx work on LW, MW and SW. On FM mode they work from 64MHz to 222MHz.
 * [PU2CLR KT0915 Arduino Library](https://pu2clr.github.io/KT0915/). The KT0915 is a full band AM (LW, MW and SW) and FM DSP receiver that can provide you a easy way to build a high quality radio with low cost.
 * 
 * For SI5351 control, this sketch uses the the Etherkit/si5351 Arduino library from Jason Milldrum (https://github.com/etherkit/Si5351Arduino);
 * For encoder control, this sketch uses the Rotary Encoder Class implementation from Ben Buxton (the source code is included together with this sketch)
 * 
 * Arduino Pro Mini 3.3V (8MHz) and SI5351 breakout wire up
 *
 * | Device name        | Device Pin / Description  |  Arduino Pin  |
 * | ----------------   | --------------------      | ------------  |
 * |   OLED             |                           |               |
 * |                    | SDA                       |     A4        |
 * |                    | SCL                       |     A5        |
 * |   SI5351           |                           |               |
 * |                    | SDIO (pin 18)             |     A4        |
 * |                    | SCLK (pin 17)             |     A5        |
 * |   Button           |                           |               |
 * |                    | Step Switch               |      4        |
 * |   Encoder          |                           |               |
 * |                    | A                         |      2        |
 * |                    | B                         |      3        |
 * 
 * See https://github.com/etherkit/Si5351Arduino  and know how to calibrate your Si5351
 * Author: Ricardo Lima Caratti 2020
*/

#include <si5351.h>
#include <Wire.h>
#include "Rotary.h"
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

// Enconder PINs
#define ENCODER_PIN_A 2 // Encoder pin A
#define ENCODER_PIN_B 3 // Encoder pin B
#define BUTTON_STEP 4   // ENCODER button or regular push button

// OLED Diaplay constants
#define I2C_ADDRESS 0x3C
#define RST_PIN -1 // Define proper RST_PIN if required.

// Change this value bellow  (CORRECTION_FACTOR) to 0 if you do not know the correction factor of your Si5351A.
#define CORRECTION_FACTOR 80000 // See how to calibrate your Si5351A (0 if you do not want).

// VFO range for this project is 3000 KHz to 30000 KHz (3MHz to 30MHz).
#define DEFAULT_VFO 800000000LU // VFO center frequency
#define MIN_VFO 300000000LU     // VFO min. frequency 3Mhz
#define MAX_VFO 3000000000LU    // VFO max. frequency 30MHz

// Encoder controller
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

// OLED - Declaration for a SSD1306 display connected to I2C (SDA, SCL pins)
SSD1306AsciiAvrI2c display;

// The Si5351 instance.
Si5351 si5351;

// Struct for step
typedef struct
{
  char *name; // step label: 50Hz, 10Hz, 500Hz and 100KHz
  long value; // Frequency value (unit 0.01Hz See documentation) to increment or decrement
} Step;

// Steps database. You can change the Steps and numbers of steps here if you need.
Step step[] = {
    {"100Hz", 10000}, // Minimum Frequency step (incremente or decrement) 100Hz
    {"500Hz", 50000},
    {"1KHz ", 100000},
    {"5KHz ", 500000}}; // Maximum frequency step 5Khz

// Calculate the index of last position of step[] array
const int lastStepVFO = (sizeof step / sizeof(Step)) - 1;
volatile long currentStep = 0;

bool isFreqChanged = true;
bool clearDisplay = false;

uint64_t vfoFreq = DEFAULT_VFO;
uint64_t vfoLastValue;

// Encoder control variables
volatile int encoderCount = 0;

void setup()
{
  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  // Si5351 contrtolers pins

  pinMode(BUTTON_STEP, INPUT_PULLUP);

  // Initiating the OLED Display
  display.begin(&Adafruit128x64, I2C_ADDRESS);
  display.setFont(Adafruit5x7);
  display.set2X();
  display.clear();
  display.print("\nShortwave");
  display.print("\nTransmitter");
  display.print("\n\nBY PU2CLR");

  delay(3000);
  display.clear();
  showStatus();
  // Initiating the Signal Generator (si5351)
  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  // Adjusting the frequency (see how to calibrate the Si5351 - example si5351_calibration.ino)
  si5351.set_correction(CORRECTION_FACTOR, SI5351_PLL_INPUT_XO);
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.set_freq(vfoFreq, SI5351_CLK0); // Start CLK0 (VFO)

  // Disable CLK 1 and 2 outputs
  si5351.output_enable(SI5351_CLK1, 0);
  si5351.output_enable(SI5351_CLK2, 0);
  si5351.update_status();

  delay(500);

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  delay(1000);
}

// Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
void rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
  {
    if (encoderStatus == DIR_CW)
    {
      encoderCount = 1;
    }
    else
    {
      encoderCount = -1;
    }
  }
}

// Show Signal Generator Information
// Verificar setCursor() em https://github.com/greiman/SSD1306Ascii/issues/53
void showStatus()
{
  double vfo = vfoFreq / 100000.0;

  // display.setCursor(0,0)
  // display.clear();
  display.set2X();
  display.setCursor(0, 0);
  display.print(" ");
  display.print(vfo);
  display.print(" KHz");

  display.print("\n\nStep: ");
  display.print(step[currentStep].name);
}

// Change the frequency (increment or decrement)
// direction parameter is 1 (clockwise) or -1 (counter-clockwise)
void changeFreq(int direction)
{
  vfoFreq += step[currentStep].value * direction; // currentStep * direction;
  // Check the VFO limits
  if (vfoFreq > MAX_VFO )
    vfoFreq = MIN_VFO;
  else if (vfoFreq < MIN_VFO)
    vfoFreq = MAX_VFO;

  isFreqChanged = true;
}

void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (encoderCount == 1)
      changeFreq(1);
    else
      changeFreq(-1);
    encoderCount = 0;
  }

  if (digitalRead(BUTTON_STEP) == LOW) {
    currentStep = (currentStep < lastStepVFO) ? (currentStep + 1) : 0;
    showStatus();
  }

  if (isFreqChanged)
  {
    si5351.set_freq(vfoFreq, SI5351_CLK0);
    showStatus();
    isFreqChanged = false;
  }
  delay(50);
}
