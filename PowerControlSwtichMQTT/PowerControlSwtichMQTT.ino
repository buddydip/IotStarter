#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>


//Wi-fi connection
const char *ssid = "MORPHEOUS";
const char *password = "whatever2020";

//MQTT Connection
const char *mqtt_server = "Smarty.local";
const int mqtt_port = 1883;
const char *mqtt_topic = "smarty/switchcontrol";

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

//PIN Switch Mapping
const int Switch1 = 5;
const int Switch2 = 4;
const int Switch3 = 14;
const int Switch4 = 12;
const char *nodeID = "POWERCONTROL1";
  
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
 
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(14, HIGH);
  digitalWrite(12, HIGH);


  // Connect to WiFi network
  Serial.print("Connecting to ");
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
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");


 // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "<hostname>.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(nodeID, WiFi.localIP())) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  
  //connect to MQTT broker
  connectMQTT();
}

int connectMQTT() 
{
  Serial.print("Connecting to MQTT Broker ");
  Serial.print("Connecting to MQTT Broker ");
  Serial.print(mqtt_server);
  Serial.println(" at port "+mqtt_port);
 
  //connect to MQTT server
  mqttclient.setClient(wifimqClient);
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(MQTTcallback);

  if (mqttclient.connect(nodeID))
    {
      Serial.println("Connected to MQTT Broker");
      
      boolean subs = mqttclient.subscribe(mqtt_topic);
      
      Serial.print("Subscribed to topic "); 
      Serial.println(mqtt_topic); 
      String msg = String("Connected. State : "+String(mqttclient.state())+" Topic Subscribed :"+ String(subs) +"  Topic : " + String(mqtt_topic));
      mqttclient.publish("debugsmarty", msg.c_str());    
     
    }
    else
    {
      Serial.print("Failed to connect to MQTT broker with state ");
      Serial.println(mqttclient.state());
      String msg = String("Not Connected : "+mqttclient.state());
      mqttclient.publish("debugsmarty", msg.c_str());    
      delay(2000);
    }
  return mqttclient.state();
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.println("Message:");
  String message;
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];
  }
  Serial.flush();
  Serial.println(message);
  
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, message);

  // Test if parsing succeeds.
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();
  String switchID = doc["SwitchID"];
  const char *stateTime = doc["Time"];
  int switchState = doc["SwitchState"];
 

  //Check swicth to toggle
  if(switchID.equals("Switch1"))
  {
      digitalWrite(Switch1, !switchState);
  }
  if(switchID.equals("Switch2"))  
  {
    digitalWrite(Switch2, !switchState);
  }
  if(switchID.equals("Switch3"))  
  {
    digitalWrite(Switch3, !switchState);
  }
  if(switchID.equals("Switch4"))  
  {
    digitalWrite(Switch4, !switchState);
  }
     
  mqttclient.publish("debugsmarty", ("Switch :"+switchID+"  State  :"+switchState+"  Time : "+stateTime).c_str());    
  
}

void loop() {

  MDNS.update();

  
  // Check if a client has connected
  WiFiClient wifiClient = server.available();
  
  //check MQTT Connection
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();
  timeClient.update();
  
  while (!wifiClient.available()) {
    delay(1);
    if (!wifiClient) {
  
      return;
    }
  }

  // Read the first line of the request
  String request = wifiClient.readStringUntil('\r');
  Serial.println(request);
  wifiClient.flush();

  //Check amd control switch state
  if (request.indexOf("/light1on") > 0)  {
    digitalWrite(5, LOW);
    controlState ="{\"SwitchID\":\"Switch1\",\"SwitchState\":1, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light1off") > 0)  {
    digitalWrite(5, HIGH);
    controlState ="{\"SwitchID\":\"Switch1\",\"SwitchState\":0, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());
  }

  if (request.indexOf("/light2on") > 0)  {
    digitalWrite(4, LOW);
    controlState ="{\"SwitchID\":\"Switch2\",\"SwitchState\":1, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light2off") > 0)  {
    digitalWrite(4, HIGH);
    controlState ="{\"SwitchID\":\"Switch2\",\"SwitchState\":0, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light3on") > 0)  {
    digitalWrite(14, LOW);
    controlState ="{\"SwitchID\":\"Switch3\",\"SwitchState\":1, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light3off") > 0)  {
    digitalWrite(14, HIGH);
    controlState ="{\"SwitchID\":\"Switch3\",\"SwitchState\":0, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light4on") > 0)  {
    digitalWrite(12, LOW);
    controlState ="{\"SwitchID\":\"Switch4\",\"SwitchState\":1, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }
  if (request.indexOf("/light4off") > 0)  {
    digitalWrite(12, HIGH);
    controlState ="{\"SwitchID\":\"Switch4\",\"SwitchState\":0, \"Time\":\""+timeClient.getFormattedTime()+"\"}";
    mqttclient.publish(mqtt_topic, controlState.c_str());

  }

 
  // Return the response
  wifiClient.println("HTTP/1.1 200 OK");
  wifiClient.println("Content-Type: text/html");
  wifiClient.println(""); //  do not forget this one
  wifiClient.println("<!DOCTYPE HTML>");
  wifiClient.println("<html>");
  wifiClient.println("<head>");
  wifiClient.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
  wifiClient.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
  wifiClient.println("</head>");
  wifiClient.println("<body bgcolor = \"#f7e6ec\">");
  wifiClient.println("<style>");
  wifiClient.println(".button {");
  wifiClient.println("background-color: #008CBA;");
  wifiClient.println("border: none;");
  wifiClient.println("color: white;");
  wifiClient.println("padding: 15px 32px;");
  wifiClient.println("text-align: center;");
  wifiClient.println("text-decoration: none;");
  wifiClient.println("display: inline-block;");
  wifiClient.println("font-size: 16px;");
  wifiClient.println("margin: 4px 2px;");
  wifiClient.println("cursor: pointer;");
  wifiClient.println("}");
  wifiClient.println("</style>");
  wifiClient.println("<hr/><hr>");
  wifiClient.println("<h4><center> Smart Switch Control </center></h4>");
  wifiClient.println("<hr/><hr>");
  wifiClient.println("<br><br>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 1</B>");
  wifiClient.println("<a href=\"/light1on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light1off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 2</b>");
  wifiClient.println("<a href=\"/light2on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light2off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 3</b>");
  wifiClient.println("<a href=\"/light3on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light3off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 4</b>");
  wifiClient.println("<a href=\"/light4on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light4off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<table border=\"5\">");
  wifiClient.println("<tr>");
  if (!digitalRead(5))
  {
    wifiClient.print("<td bgcolor='red'>Switch 1 is ON</td>");
 

  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 1 is OFF</td>");
    
  }

  wifiClient.println("<br />");

  if (!digitalRead(4))
  {
    wifiClient.print("<td bgcolor='red'>Switch 2 is ON</td>");
  
  }
  else
  {

    wifiClient.print("<td bgcolor='green'>Switch 2 is OFF</td>");
   
  }


  if (!digitalRead(14))

  {
    wifiClient.print("<td bgcolor='red'>Switch 3 is ON</td>");
    
  }

  else

  {
    wifiClient.print("<td bgcolor='green'>Switch 3 is OFF</td>");
   
  }


  if (!digitalRead(12))


  {


    wifiClient.print("<td bgcolor='red'>Switch 4 is ON</td>");
   
  }


  else


  {


    wifiClient.print("<td bgcolor='green'>Switch 4 is OFF</td>");
   

  }

  wifiClient.println("</tr>");
  wifiClient.println("</table>");
  wifiClient.println("<BR>");
  wifiClient.println("</center>");
  wifiClient.println("</html>");
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");

}
