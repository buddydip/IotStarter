#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//Wi-fi connection
const char *ssid = "MORPHEOUS";
const char *password = "whatever2020";

//MQTT Connection
const char *mqtt_server = "192.168.1.8";
const int mqtt_port = 1883;
const char *mqtt_topic = "test";

//JSON message for switch control
String controlState = "";
StaticJsonDocument<200> doc;


//Wifiserver instance at port 80
WiFiServer server(80);

//MQTT client at port 1883
WiFiClient wifimqClient;
PubSubClient mqttclient;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
 
void setup() {
  Serial.begin(9600);
  Serial.flush();
  delay(10);
  Serial.println();
  
  timeClient.begin();
  
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(1, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(1, LOW);

  // Connect to WiFi network
  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println("Use this URL to connect: ");
  Serial.println("http://"+WiFi.localIP());
 

  connectMQTT();
}

int connectMQTT() 
{
  //connect to MQTT server
  mqttclient.setClient(wifimqClient);
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(MQTTcallback);

  if (mqttclient.connect("ESP8266", "pi", "meripi123"))
    {
      Serial.println("connected");
      mqttclient.subscribe(mqtt_topic); 
    }
    else
    {
      Serial.println("failed with state "+ mqttclient.state());
      delay(2000);
    }
  return mqttclient.state();
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
  digitalWrite(1, HIGH);
  
  Serial.println("Message received in topic: ");
  Serial.println(topic);
  Serial.println("Message:");
  String message;
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  if (message == "on") 
  {
    //digitalWrite(1, HIGH);
  }
  else if (message == "off") 
  {
   // digitalWrite(1, LOW);
  }
  Serial.println();
  Serial.println("-----------------------");



  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, (char *)payload);

  // Test if parsing succeeds.
  if (error) {
    Serial.println("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();
  String switchID = doc["SwitchID"];
  long stateTime = doc["Time"];
  int switchState = doc["SwitchState"];
 

  // Print values.
  Serial.println(switchID);
  Serial.println(stateTime);
  Serial.println(switchState);

  if (switchID.equals("Switch1") && switchState == 1)  {
    digitalWrite(5, LOW);
    controlState ="{\"SwitchID\":\"Switch1\",\"SwitchState\":1, \"Time\":"+timeClient.getFormattedTime()+"}";
    //mqtt_payload = controlState;
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
   if (switchID.equals("Switch1") && switchState == 0)  {
    digitalWrite(5, HIGH);
    controlState ="{\"SwitchID\":\"Switch1\",\"SwitchState\":0, \"Time\":"+timeClient.getFormattedTime()+"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());
  }
      
  digitalWrite(1, LOW);
}

void loop() {

//check MQTT Connection
/*
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();

  */
  
  timeClient.update();
  

}
