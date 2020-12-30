//Arduino Sound Detection Sensor Module

int soundDetectedPin = 10; // Use Pin 10 as our Input
int ledPin = 12; // Use Pin 12 as our output
int soundDetectedVal = HIGH; // This is where we record our Sound Measurement
boolean bAlarm = false;

unsigned long lastSoundDetectTime; // Record the time that we measured a sound


int soundAlarmTime = 500; // Number of milli seconds to keep the sound alarm high


void setup ()
{
  Serial.begin(9600);  
  pinMode (soundDetectedPin, INPUT) ; // input from the Sound Detection Module
  pinMode (ledPin, OUTPUT) ; // output to LED
}
void loop ()
{
  soundDetectedVal = digitalRead (soundDetectedPin) ; // read the sound alarm time
  Serial.println(soundDetectedVal);
  if (soundDetectedVal == LOW) // If we hear a sound
  {
  
    lastSoundDetectTime = millis(); // record the time of the sound alarm
    // The following is so you don't scroll on the output screen
    if (!bAlarm){
      Serial.println("LOUD");
      // turn LED on:
      digitalWrite(ledPin, HIGH);
      bAlarm = true;
    }
  }
  else
  {
    if( (millis()-lastSoundDetectTime) > soundAlarmTime  &&  bAlarm){
      Serial.println("quiet");
       // turn LED off:
      digitalWrite(ledPin, LOW);
      bAlarm = false;
    }
  }
}
