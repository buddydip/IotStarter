
#include <dht.h>

#define DHT11_PIN 7


dht DHT;
double temperature;
double humidity;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
 int chk = DHT.read11(DHT11_PIN); 
  temperature = DHT.temperature;
  humidity = DHT.humidity;

  Serial.println("Temperature :"+String(temperature)+"  Humidity :"+String(humidity));
  delay(1000);
}
