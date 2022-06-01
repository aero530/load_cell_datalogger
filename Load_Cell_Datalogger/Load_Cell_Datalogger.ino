/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <SD.h>
#include "HX711.h"

HX711 scale;

const uint8_t ledPin =  13;
const uint8_t buttonPin =  6;
const uint8_t dataPin = 3;
const uint8_t clockPin = 2;
const uint8_t chipSelect = 5;

uint32_t start, stop;
volatile float f;

bool record = false;
int fileNumber = 0;

char fileName[13] = "data_000.csv";

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin,LOW);
  pinMode(buttonPin, INPUT_PULLUP);
  
  scale.begin(dataPin, clockPin);

  // TODO find a nice solution for this calibration..
  // loadcell factor 20 KG
  // scale.set_scale(127.15);
  // loadcell factor 5 KG
  // output values are approx grams of force
  scale.set_scale(420.0983);
  // reset the scale to zero = 0
  scale.tare();

  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  for (int i=0; i<=999; i++) {
    sprintf(fileName, "data_%03d.csv",i);
    if (SD.exists(fileName)) {
      //Serial.println("removing existing data file");
      //SD.remove(fileName);
      fileNumber = i+1;
    }
  }
  //reset filename
  sprintf(fileName, "data_%03d.csv",fileNumber);

}

void loop() {
  // read the state of the switch into a local variable:
  int buttonReading = digitalRead(buttonPin);
  
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (buttonReading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (buttonReading != buttonState) {
      buttonState = buttonReading;

      // only toggle record if the new button state is HIGH
      if (buttonState == LOW) {
        record = !record;
        if (record == true) {
          fileNumber += 1;
          sprintf(fileName, "data_%03d.csv",fileNumber);
          Serial.print("creating file ");
          Serial.println(fileName);
          digitalWrite(ledPin, LOW);
        } else {
          digitalWrite(ledPin, HIGH);
        }
      }
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = buttonReading;

  if (record == true) {
    
    // make a string for assembling the data to log:
    String dataString = "";
    dataString += String(millis(),DEC);
    dataString += ",";
  
    f = scale.get_units(5);
    dataString += String(f);
  
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open(fileName, FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    } else {
      Serial.print("error opening ");
      Serial.println(fileName);
    }
  }
  
  delay(12);
}
