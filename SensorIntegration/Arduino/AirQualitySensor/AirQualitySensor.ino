  #include <MQ2.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>
SoftwareSerial s(5,6); // (Rx, Tx)

//JSON message for switch control
StaticJsonDocument<200> doc;

//define PINS
#define DHTPIN 13
#define MQ2_PIN A0
#define PIR_PIN 12
#define DHTTYPE DHT11   // DHT 11 

DHT dht = DHT(DHTPIN, DHTTYPE);
float temperature = 0;
float humidity = 0;
float oldtemperature = 0;
float oldhumidity = 0;
float heatIndex = 0;

//MQ135
int lpg, co, smoke;
int oldlpg, oldco, oldsmoke;
String outputString = "";
MQ2 mq2(MQ2_PIN);


boolean motion = false;
long lastMotionTime = 0;
String lastMotionTimeformattted = "";
//long motionDetectionInterval = 300000;
long motionDetectionInterval = 60000;
long delayTime = 60000;

void setup() {

  Serial.begin(115200);
  s.begin(115200);
  Serial.flush();
  
  pinMode(PIR_PIN, INPUT);
  pinMode(MQ2_PIN, INPUT);

  dht.begin();
  mq2.begin();
  
  delay(2000);
}

void loop() {

  //DTH Reading
  oldtemperature = temperature;
  oldhumidity = humidity;

   // Read the humidity in %:
  humidity = dht.readHumidity();
  // Read the temperature as Celsius:
  temperature = dht.readTemperature();


  // Check if any reads failed and exit early (to try again):
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    dht.begin();
    return;
  }

  // Compute heat index in Celsius:
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  
  //MQ2 Gas Sensor Reading
  float* values= mq2.read(true); //set it false if you don't want to print the values in the Serial
  oldlpg = lpg;
  oldco=co;
  oldsmoke=smoke;

  //lpg = values[0];
  lpg = mq2.readLPG();
  //co = values[1];
  co = mq2.readCO();
  //smoke = values[2];
  smoke = mq2.readSmoke();

  Serial.print("ESP Serial Available: ");
  Serial.println(s.available());

  Serial.print("Temperarture : ");
  Serial.println(temperature);

  Serial.print("Humidity : ");
  Serial.println(humidity);

  Serial.print("Heat Index : ");
  Serial.println(heatIndex);

  Serial.print("LPG : ");
  Serial.println(values[0]);

  Serial.print("CO2 : ");
  Serial.println(values[1]);
  
  Serial.print("Smoke : ");
  Serial.println(values[2]);

    
    if (digitalRead(PIR_PIN) == HIGH)
    {        
        motion = true;
        long nowTime = millis();
        Serial.println("Motion Detected!");
        if ((nowTime - lastMotionTime) > motionDetectionInterval)
        {
          outputString = "";
          doc["ParamName"] = "Motion";
          doc["ParamValue"] = motion;
          serializeJson(doc, outputString);
          Serial.println(outputString);
          lastMotionTime = nowTime;

          //write to serial for NodeMCU to read
          s.print(outputString);
          s.println();
          doc.clear();
          delay(1000);          
        }
    }
    else
    {
      motion = false;
    }
 
  if ((abs(oldtemperature - temperature) > 1) && !isnan(temperature))
  {
    outputString = "";
    doc["ParamName"] = "Temperature";
    doc["ParamValue"] = temperature;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
    delay(1000);    
  }


  if ((abs(oldhumidity - humidity) > 1) && !isnan(humidity))
  {
    outputString = "";
    doc["ParamName"] = "Humidity";
    doc["ParamValue"] = humidity;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
    delay(1000);
  }

  if (((abs(oldtemperature - temperature) > 1) || (abs(oldhumidity - humidity) > 1)) && !isnan(heatIndex))
  {
    outputString = "";
    doc["ParamName"] = "HeatIndex";
    doc["ParamValue"] = heatIndex;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
    delay(1000);
  }

  if (smoke != oldsmoke)
  {
    outputString = "";
    doc["ParamName"] = "Smoke";
    doc["ParamValue"] = smoke;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
    delay(1000);
  }

  if (lpg != oldlpg)
  {
    outputString = "";
    doc["ParamName"] = "LPG";
    doc["ParamValue"] = lpg;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
    delay(1000);
  }

  if (co != oldco)
  {
    outputString = "";
    doc["ParamName"] = "CO2";
    doc["ParamValue"] = co;
    serializeJson(doc, outputString);

    Serial.println(outputString);
    
    //write to serial for NodeMCU to read
    s.print(outputString);
    s.println();
    doc.clear();
  }

  delay(delayTime);
}
