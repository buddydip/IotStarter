#include <EEPROM.h>
#include <dht.h> 

#define dht_apin A0 // Analog Pin sensor is connected to 
dht DHT;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  delay(500);//Delay to let system boot 
  //Serial.println("DHT11 Humidity & temperature Sensor\n\n"); 
  delay(1000);//Wait before accessing Sensor 
}

void loop() {
  // put your main code here, to run repeatedly:
  DHT.read11(dht_apin); 
  
  Serial.print(DHT.temperature);     
  Serial.println("C  ");  
  delay(5000);//Wait 5 seconds before accessing sensor again.
  
}
