/*
  Arduino Starter Kit example
 Project 3  - Love-O-Meter

 This sketch is written to accompany Project 3 in the
 Arduino Starter Kit

 Parts required:
 1 TMP35 temperature sensor
 3 red LEDs
 3 220 ohm resistors

 Created 13 September 2012
 by Scott Fitzgerald

 http://www.arduino.cc/starterKit

 This example code is part of the public domain
 */

// named constant for the pin the sensor is connected to
const int sensorPin = A0;
// room temperature in Celcius
const float baselineTemp = 30.0;

const int hLEDPIN = 13;
const int lLEDPIN = 12;

void setup() {
  // open a serial connection to display values
  analogReference(INTERNAL);
  Serial.begin(9600);
  // set the LED pins as outputs
  // the for() loop saves some extra coding
    pinMode(hLEDPIN, OUTPUT);
    pinMode(lLEDPIN, OUTPUT);
    digitalWrite(hLEDPIN, LOW);
    digitalWrite(lLEDPIN, LOW);
}

void loop() {
  // read the value on AnalogIn pin 0
  // and store it in a variable
  int sensorVal = analogRead(sensorPin);

  // send the 10-bit sensor value out the serial port
  //Serial.print("sensor Value: ");
  //Serial.print(sensorVal);

  // convert the voltage to temperature in degrees C
  //Serial.print(", degrees C: ");
  float temperature = sensorVal/9.31;
  Serial.println(temperature);

  // if the current temperature is lower than the baseline
  // turn off all LEDs
  if (temperature < baselineTemp) {
    digitalWrite(lLEDPIN, HIGH);
    digitalWrite(hLEDPIN, LOW);
    
  } else if (temperature >= baselineTemp) {
    digitalWrite(lLEDPIN, LOW);
    digitalWrite(hLEDPIN, HIGH);
  } 
  delay(1000);
}
