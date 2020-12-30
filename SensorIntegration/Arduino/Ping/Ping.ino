  /*
 HC-SR04 Ping distance sensor:
 VCC to arduino 5v 
 GND to arduino GND
 Echo to Arduino pin 7 
 Trig to Arduino pin 8
 
 This sketch originates from Virtualmix: http://goo.gl/kJ8Gl
 Has been modified by Winkle ink here: http://winkleink.blogspot.com.au/2012/05/arduino-hc-sr04-ultrasonic-distance.html
 And modified further by ScottC here: http://arduinobasics.blogspot.com.au/2012/11/arduinobasics-hc-sr04-ultrasonic-sensor.html
 on 10 Nov 2012.
 */


#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define dLEDPin 12 // Distance LED

int maximumRange = 2000; // Maximum range needed
int minimumRange = 10; // Minimum range needed
long duration, distance; // Duration used to calculate distance
// named constant for the pin the sensor is connected to
const int sensorPin = A0;
// room temperature in Celcius
float baselineTemp = 30.0;
const int hLEDPIN = 13;

void setup() {
 Serial.begin (9600);
 analogReference(INTERNAL);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(dLEDPin, OUTPUT); // Use LED indicator (if required)
 pinMode(hLEDPIN, OUTPUT);
 
}

void loop() {
/* The following trigPin/echoPin cycle is used to determine the
 distance of the nearest object by bouncing soundwaves off of it. */ 
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
   delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 distance = (duration/29)/2;
 
 
 if (distance >= maximumRange || distance <= minimumRange){
 /* Send a negative number to computer and Turn LED ON 
 to indicate "out of range" */
 digitalWrite(dLEDPin, HIGH); 
 }
 else {
 /* Send the distance to the computer using Serial protocol, and
 turn LED OFF to indicate successful reading. */
 digitalWrite(dLEDPin, LOW); 
 }

// read the value on AnalogIn pin 0
  // and store it in a variable
  int sensorVal = analogRead(sensorPin);
  float temperature = sensorVal/9.31;

  // if the current temperature is lower than the baseline
  // turn off all LEDs
  if (temperature < baselineTemp) {
    digitalWrite(hLEDPIN, LOW);
  }
  else
  {
    digitalWrite(hLEDPIN, HIGH);
  } 

if ( Serial.available() > 0 ) {
    // read a numbers from serial port
    int count = Serial.parseInt();
    Serial.print("You have input: ");
     // print out the received number
    if (count > 0) {
        // blink the LED
        blinkLED(count);
    }
  }

  Serial.println("\"ParamName\": \"Distance\", \"ParamValue\":\""+String(distance)+"\""); 
  
   //Serial.println("\"ParamName\":\"Temperature\", \"ParamValue\":\""+String(temperature)+"\"");
  
 //Delay 100ms before next reading.
 delay(500);
}

void blinkLED(int count) {
  for (int i=0; i< count; i++) {
    digitalWrite(hLEDPIN, HIGH);
    delay(500);
    digitalWrite(hLEDPIN, LOW);
    delay(500);
  } 
}

