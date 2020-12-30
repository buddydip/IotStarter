#define SOILSENSOR_ANPIN A2


void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
float soilMoisture = analogRead(SOILSENSOR_ANPIN);
Serial.println("SoilMoisture :"+String(soilMoisture));
delay(500);
}
