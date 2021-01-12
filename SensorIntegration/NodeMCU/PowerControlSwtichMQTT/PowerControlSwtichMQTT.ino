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
const int utcOffsetInSeconds = 19800;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//PIN Switch Mapping
const int Switch1 = 5;
const int Switch2 = 4;
const int Switch3 = 14;
const int Switch4 = 12;
const int Switch5 = 16;
const int Switch6 = 2;
const int Switch7 = 10;
const int Switch8 = 13;

const char *nodeID = "POWERCONTROL1";
  
void setup() {
  Serial.begin(9600);
  Serial.flush();
  delay(10);
  Serial.println();
  
  timeClient.begin();
  
  pinMode(Switch1, OUTPUT);
  pinMode(Switch2, OUTPUT);
  pinMode(Switch3, OUTPUT);
  pinMode(Switch4, OUTPUT);
  pinMode(Switch5, OUTPUT);
  pinMode(Switch6, OUTPUT);
  pinMode(Switch7, OUTPUT);
  pinMode(Switch8, OUTPUT);
 
  digitalWrite(Switch1, HIGH);
  digitalWrite(Switch2, HIGH);
  digitalWrite(Switch3, HIGH);
  digitalWrite(Switch4, HIGH);
  digitalWrite(Switch5, HIGH);
  digitalWrite(Switch6, HIGH);
  digitalWrite(Switch7, HIGH);
  digitalWrite(Switch8, HIGH);

  
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
 

  //Check switch to toggle
  if(switchID.equals("Switch1") && (digitalRead(Switch1) == switchState))
  {
      digitalWrite(Switch1, !switchState);
  }
  if(switchID.equals("Switch2") && (digitalRead(Switch2) == switchState))  
  {
    digitalWrite(Switch2, !switchState);
  }
  if(switchID.equals("Switch3") && (digitalRead(Switch3) == switchState))  
  {
    digitalWrite(Switch3, !switchState);
  }
  if(switchID.equals("Switch4") && (digitalRead(Switch4) == switchState))  
  {
    digitalWrite(Switch4, !switchState);
  }

  if(switchID.equals("Switch5") && (digitalRead(Switch5) == switchState))  
  {
    digitalWrite(Switch5, !switchState);
  }
  
    if(switchID.equals("Switch6") && (digitalRead(Switch6) == switchState))  
  {
    digitalWrite(Switch6, !switchState);
  }
  
    if(switchID.equals("Switch7") && (digitalRead(Switch7) == switchState))  
  {
    digitalWrite(Switch7, !switchState);
  }
  
    if(switchID.equals("Switch8") && (digitalRead(Switch8) == switchState))  
  {
    digitalWrite(Switch8, !switchState);
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
    
    digitalWrite(Switch1, LOW);
    
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch1";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();


  }
  if (request.indexOf("/light1off") > 0)  {
    digitalWrite(Switch1, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch1";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/light2on") > 0)  {
    digitalWrite(Switch2, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch2";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }
  if (request.indexOf("/light2off") > 0)  { 
    
    digitalWrite(Switch2, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch2";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }
  if (request.indexOf("/light3on") > 0)  {
    
    digitalWrite(Switch3, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch3";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }
  if (request.indexOf("/light3off") > 0)  {
    
    digitalWrite(Switch3, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch3";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }
  if (request.indexOf("/light4on") > 0)  {
    
    digitalWrite(Switch4, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch4";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }
  if (request.indexOf("/light4off") > 0)  {

    digitalWrite(Switch4, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch4";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

    if (request.indexOf("/light5on") > 0)  {
    
    digitalWrite(Switch5, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch5";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

  if (request.indexOf("/light5off") > 0)  {

    digitalWrite(Switch5, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch5";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

    if (request.indexOf("/light6on") > 0)  {
    
    digitalWrite(Switch6, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch6";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

  if (request.indexOf("/light6off") > 0)  {

    digitalWrite(Switch6, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch6";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }


     if (request.indexOf("/light7on") > 0)  {
    
    digitalWrite(Switch7, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch7";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

  if (request.indexOf("/light7off") > 0)  {

    digitalWrite(Switch7, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch7";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

    if (request.indexOf("/light8on") > 0)  {
    
    digitalWrite(Switch8, LOW);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch8";
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

  }

  if (request.indexOf("/light8off") > 0)  {

    digitalWrite(Switch8, HIGH);
    doc.clear();
    controlState="";
    doc["SwitchID"] = "Switch8";
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();

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
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 5</b>");
  wifiClient.println("<a href=\"/light5on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light5off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 6</b>");
  wifiClient.println("<a href=\"/light6on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light6off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 7</b>");
  wifiClient.println("<a href=\"/light7on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light7off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<b>Switch 8</b>");
  wifiClient.println("<a href=\"/light8on\"\"><button class=\"button\">Turn On </button></a>");
  wifiClient.println("<a href=\"/light8off\"\"><button class=\"button\">Turn Off </button></a><br />");
  wifiClient.println("</center>");
  wifiClient.println("<br><br>");
  wifiClient.println("<br><br>");
  wifiClient.println("<center>");
  wifiClient.println("<table border=\"5\">");
  wifiClient.println("<tr>");
  if (!digitalRead(Switch1))
  {
    wifiClient.print("<td bgcolor='red'>Switch 1 is ON</td>");
 

  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 1 is OFF</td>");
    
  }

  wifiClient.println("<br />");

  if (!digitalRead(Switch2))
  {
    wifiClient.print("<td bgcolor='red'>Switch 2 is ON</td>");
  
  }
  else
  {

    wifiClient.print("<td bgcolor='green'>Switch 2 is OFF</td>");
   
  }


  if (!digitalRead(Switch3))

  {
    wifiClient.print("<td bgcolor='red'>Switch 3 is ON</td>");
    
  }

  else

  {
    wifiClient.print("<td bgcolor='green'>Switch 3 is OFF</td>");
   
  }


  if (!digitalRead(Switch4))
  {
    wifiClient.print("<td bgcolor='red'>Switch 4 is ON</td>");
  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 4 is OFF</td>");
  }

  wifiClient.println("</tr><tr>");
 

  if (!digitalRead(Switch5))
  {
    wifiClient.print("<td bgcolor='red'>Switch 5 is ON</td>");
  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 5 is OFF</td>");
  }


  if (!digitalRead(Switch6))
  {
    wifiClient.print("<td bgcolor='red'>Switch 6 is ON</td>");
  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 6 is OFF</td>");
  }


  if (!digitalRead(Switch7))
  {
    wifiClient.print("<td bgcolor='red'>Switch 7 is ON</td>");
  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 7 is OFF</td>");
  }


  if (!digitalRead(Switch8))
  {
    wifiClient.print("<td bgcolor='red'>Switch 8 is ON</td>");
  }
  else
  {
    wifiClient.print("<td bgcolor='green'>Switch 8 is OFF</td>");
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

String getCurrentDateTime()
{
  unsigned long epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  


  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  int currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDateTime = String(currentYear) + ":" + String(currentMonth) + ":" + String(monthDay)+"T"+formattedTime;
  Serial.print("Current date: ");
  Serial.println(currentDateTime);
  return currentDateTime; 
 
}
