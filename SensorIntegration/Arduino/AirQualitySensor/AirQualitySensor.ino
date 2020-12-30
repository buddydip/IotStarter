
/*
 Standalone Sketch to use with a Arduino Fio and a
 Sharp Optical Dust Sensor GP2Y1010AU0F
 
 Blog: http://arduinodev.woofex.net/2012/12/01/standalone-sharp-dust-sensor/
 Code: https://github.com/Trefex/arduino-airquality/
 
 For Pin connections, please check the Blog or the github project page
 Authors: Cyrille Médard de Chardon (serialC), Christophe Trefois (Trefex)
 Changelog:
   2012-Dec-01:  Cleaned up code
 
 This work is licensed under the
 Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/3.0/
 or send a letter to Creative Commons, 444 Castro Street, Suite 900,
 Mountain View, California, 94041, USA.
*/

#include <dht.h>
#include <MQ135.h>

//define PINS
#define DHT11_PIN 6
#define DUSTSENSOR_PIN A0
#define DUSTDIG_PIN 7

#define RAINSENSOR_ANPIN A1
#define RAINSENSOR_DGPIN 2

#define SOILSENSOR_ANPIN A2
 
#define MQ135SENSOR_ANPIN A3
#define MQ135SENSOR_DGPIN 8

//Dust sensor  
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
int dustMeasured;
float voMeasured = 0;
double calcVoltage = 0;
float dustDensity = 0;
float olddustDensity = 0;

//DTH
dht DHT;
float temperature;
float humidity;
float oldtemperature;
float oldhumidity;

//Rain Sensor
float nRainVal;
boolean bIsRaining = false;
float noldRainVal;
boolean boldIsRaining = false;

//Soil Moisture Sensor
float soilMoisture; 
float oldsoilMoisture; 

//MQ135
float ppm;
float oldppm;

String outputString;

void setup(){
  Serial.begin(9600);  
  pinMode(DUSTDIG_PIN,OUTPUT);
  pinMode(RAINSENSOR_DGPIN,INPUT);
  pinMode(DHT11_PIN,INPUT);
  
}
 
void loop(){


 //Dust Sensor Reading
  olddustDensity = dustDensity;

  digitalWrite(DUSTDIG_PIN,LOW); // power on the LED
  delayMicroseconds(samplingTime);
 
  voMeasured = analogRead(DUSTSENSOR_PIN); // read the dust value
 
  delayMicroseconds(deltaTime);
  digitalWrite(DUSTDIG_PIN,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
  
  // 0 - 3.3V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (5.0/1024.0);
 
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 0.17 * calcVoltage - 0.1;

  

//DTH Reading
 oldtemperature = temperature;
 oldhumidity = humidity;
 
 int chk = DHT.read11(DHT11_PIN); 
  temperature = DHT.temperature;
  humidity = DHT.humidity;

//Rain Sensor Reading
  noldRainVal = nRainVal;
  boldIsRaining = bIsRaining;
  
  nRainVal = analogRead(RAINSENSOR_ANPIN);
  nRainVal = ((1023- nRainVal)/1023)*100;
  bIsRaining = !(digitalRead(RAINSENSOR_DGPIN));
  

//Soil Moisture Sensor Reading
oldsoilMoisture = soilMoisture;
soilMoisture = analogRead(SOILSENSOR_ANPIN);
soilMoisture = ((1023- soilMoisture)/1023)*100;

//MQ135 Gas Sensor Reading
oldppm = ppm;
MQ135 gasSensor = MQ135(MQ135SENSOR_ANPIN);
float rzero = gasSensor.getRZero();
ppm = gasSensor.getPPM();
float gasDigital = digitalRead(MQ135SENSOR_DGPIN);


if(dustDensity != olddustDensity)
{
  outputString = "\"ParamName\": \"airDustDensity\",\"ParamValue\": \""+ String(dustDensity)+"\""; 
  Serial.println(outputString);
}
  
if(oldtemperature != temperature)
{
  outputString = "\"ParamName\": \"airTemperature\",\"ParamValue\": \""+ String(temperature)+"\""; 
  Serial.println(outputString);
}


if(oldhumidity != humidity)
{
  outputString = "\"ParamName\": \"airHumidity\",\"ParamValue\": \""+ String(humidity)+"\""; 
  Serial.println(outputString);
}

if(noldRainVal != nRainVal)
{
  outputString = "\"ParamName\": \"rainMeasure\",\"ParamValue\": \""+ String(nRainVal)+"\""; 
  Serial.println(outputString);
}


if(boldIsRaining != bIsRaining)
{
  outputString = "\"ParamName\": \"isRaining\",\"ParamValue\": \""+ String(bIsRaining)+"\""; 
  Serial.println(outputString);
}


if(oldsoilMoisture != soilMoisture)
{
  outputString = "\"ParamName\": \"soilMoisture\",\"ParamValue\": \""+ String(soilMoisture)+"\""; 
  Serial.println(outputString);
}


if(oldppm != ppm)
{
  outputString = "\"ParamName\": \"gasValue\",\"ParamValue\": \""+ String(ppm)+"\""; 
  Serial.println(outputString);
}  

//Print to serial
//Serial.println("\"airDustDensity\": \""+ String(dustDensity) +"\", \"airTemperature\":\""+String(temperature)+"\", \"airHumidity\":\""+String(humidity)+"\", \"isRaining\":\""+String(bIsRaining)+"\", \"rainMeasure\":\""+String(nRainVal)+"\", \"soilHumidity\":\""+String(soilMoisture)+"\", \"gasValue\":\""+String(ppm)+"\""); 
 
delay(1000);
}

