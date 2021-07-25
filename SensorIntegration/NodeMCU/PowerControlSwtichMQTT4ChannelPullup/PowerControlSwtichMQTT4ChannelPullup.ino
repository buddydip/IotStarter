#include <ArduinoOTA.h>
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
//const char *mqtt_topic = "smarty/switchcontrol/library";
//const char *mqtt_topic = "smarty/switchcontrol/stairlanding";
//const char *mqtt_topic = "smarty/switchcontrol/mainstudy";
//const char *mqtt_topic = "smarty/switchcontrol/corridor";
//const char *mqtt_topic = "smarty/switchcontrol/upperbathroom";
const char *mqtt_topic = "smarty/switchcontrol/veranda";


const char *nodeID = "VERANDA";
//const char *nodeID = "STAIRLANDING";
//const char *nodeID = "MAINSTUDY";
//const char *nodeID = "LIBRARY";
//const char *nodeID = "CORRIDOR";
//const char *nodeID = "UPPERBATHROOM";

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

//PIN Button Mapping
//const int Button1 = 2;
const int Button1 = 10;
const int Button2 = 0;
const int Button3 = 13;
const int Button4 = 3;

//Button State
int Button1State = 1;
int Button2State = 1;
int Button3State = 1;
int Button4State = 1;


//Switch name
String switch1name = String(nodeID) + ".Switch1";
String switch2name = String(nodeID) + ".Switch2";
String switch3name = String(nodeID) + ".Switch3";
String switch4name = String(nodeID) + ".Switch4";



void connectWifi()
{
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

}


void enableOTA()
{

  // Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(nodeID);

  // No authentication by default
  ArduinoOTA.setPassword("zyxcba123");

  // Password can be set with it's md5 value as well
  //ArduinoOTA.setPasswordHash("b0dc539ad687b8ed4da9e44629b778ea");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();          //OTA initialization 
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());     // Display the IP address of the ESP on the serial monitor
}


//connect to MQTT
int connectMQTT()
{
  Serial.print("Connecting to MQTT Broker ");
  Serial.print("Connecting to MQTT Broker ");
  Serial.print(mqtt_server);
  Serial.println(" at port " + mqtt_port);

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
    String msg = String("Connected. State : " + String(mqttclient.state()) + " Topic Subscribed :" + String(subs) + "  Topic : " + String(mqtt_topic));
  }
  else
  {
    Serial.print("Failed to connect to MQTT broker with state ");
    Serial.println(mqttclient.state());
    String msg = String("Not Connected : " + mqttclient.state());
    delay(2000);
  }
  return mqttclient.state();
}


//callback function for MQTT message
void MQTTcallback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
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
  if (switchID.equals(switch1name) && (digitalRead(Switch1) == switchState))
  {
    digitalWrite(Switch1, !switchState);
  }
  if (switchID.equals(switch2name) && (digitalRead(Switch2) == switchState))
  {
    digitalWrite(Switch2, !switchState);
  }
  if (switchID.equals(switch3name) && (digitalRead(Switch3) == switchState))
  {
    digitalWrite(Switch3, !switchState);
  }
  if (switchID.equals(switch4name) && (digitalRead(Switch4) == switchState))
  {
    digitalWrite(Switch4, !switchState);
  }
}


//toggle by manual switch
void toggleSwitch(int switchID, String switchName)
{
  int switchState = digitalRead(switchID);
  digitalWrite(switchID, !switchState);

  doc.clear();
  controlState = "";
  doc["SwitchID"] = switchName;
  doc["SwitchState"] = switchState;
  doc["Time"] = getCurrentDateTime();
  serializeJson(doc, controlState);
  Serial.println(controlState);

  mqttclient.publish(mqtt_topic, controlState.c_str());
  doc.clear();
}



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

  pinMode(Button1, INPUT_PULLUP);
  pinMode(Button2, INPUT_PULLUP);
  pinMode(Button3, INPUT_PULLUP);
  pinMode(Button4, INPUT_PULLUP);

  digitalWrite(Switch1, HIGH);
  digitalWrite(Switch2, HIGH);
  digitalWrite(Switch3, HIGH);
  digitalWrite(Switch4, HIGH);


  //connect to Wifi
  connectWifi();

  //enable OTA 
  enableOTA();

  //connect to MQTT broker
  connectMQTT();

}


void loop() {

  if((WiFi.status() != WL_CONNECTED))
  { 
    connectWifi();
  }
  
  MDNS.update();

  Serial.print("MQTT Connection State : ");
  Serial.println(mqttclient.state());
  //check MQTT Connection
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();
  timeClient.update();

  ArduinoOTA.handle();

  if ((digitalRead(Button1) != Button1State))
  {
    toggleSwitch(Switch1, switch1name);
    Button1State = digitalRead(Button1);
    delay(200);
    return;
  }
  if (digitalRead(Button2) != Button2State)
  {
    toggleSwitch(Switch2, switch2name);
    Button2State = digitalRead(Button2);
    delay(200);
    return;
  }
  if (digitalRead(Button3) != Button3State)
  {
    toggleSwitch(Switch3, switch3name);
    Button3State = digitalRead(Button3);
    delay(200);
    return;
  }
  if (digitalRead(Button4) != Button4State)
  {
    toggleSwitch(Switch4, switch4name);
    Button4State = digitalRead(Button4);
    delay(200);
    return;
  }


  // Check if a client has connected
  WiFiClient wifiClient = server.available();
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
  if (request.indexOf("/switch1on") > 0)  {

    digitalWrite(Switch1, LOW);
    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch1name;
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch1off") > 0)  {
    digitalWrite(Switch1, HIGH);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch1name;
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch2on") > 0)  {
    digitalWrite(Switch2, LOW);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch2name;
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch2off") > 0)  {

    digitalWrite(Switch2, HIGH);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch2name;
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch3on") > 0)  {

    digitalWrite(Switch3, LOW);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch3name;
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch3off") > 0)  {

    digitalWrite(Switch3, HIGH);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch3name;
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch4on") > 0)  {

    digitalWrite(Switch4, LOW);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch4name;
    doc["SwitchState"] = 1;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  if (request.indexOf("/switch4off") > 0)  {

    digitalWrite(Switch4, HIGH);

    doc.clear();
    controlState = "";
    doc["SwitchID"] = switch4name;
    doc["SwitchState"] = 0;
    doc["Time"] = getCurrentDateTime();
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
  }

  // Display the Page
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
  wifiClient.println(".redbutton {");
  wifiClient.println("background-color: #ff0000;");
  wifiClient.println("border: 2px solid black;");
  wifiClient.println("color: white;");
  wifiClient.println("padding: 15px 32px;");
  wifiClient.println("text-align: center;");
  wifiClient.println("text-decoration: none;");
  wifiClient.println("display: inline-block;");
  wifiClient.println("font-size: 16px;");
  wifiClient.println("margin: 4px 2px;");
  wifiClient.println("cursor: pointer;");
  wifiClient.println("}");

  wifiClient.println(".greenbutton {");
  wifiClient.println("background-color: #00ff75;");
  wifiClient.println("border: 2px solid black;");
  wifiClient.println("color: black;");
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
  wifiClient.println("<h4><center> Smart Switch Control - " + String(nodeID) + "</center></h4>");
  wifiClient.println("<hr/><hr>");
  wifiClient.println("<center>");
  wifiClient.println("<table border=\"1\" width=\"100%\">");
  wifiClient.println("<tr>");


  if (!digitalRead(Switch1))
  {
    wifiClient.println("<td><b>Switch 1</B><BR><a href=\"/switch1off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 1 is ON<BR><BR></td>");
  }
  else
  {
    wifiClient.println("<td><b>Switch 1</B><BR><a href=\"/switch1on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 1 is OFF<BR><BR></td>");
  }

  if (!digitalRead(Switch2))
  {
    wifiClient.println("<td><b>Switch 2</B><BR><a href=\"/switch2off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 2 is ON<BR><BR></td>");
  }
  else
  {

    wifiClient.println("<td><b>Switch 2</B><BR><a href=\"/switch2on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 2 is OFF<BR><BR></td>");
  }


  if (!digitalRead(Switch3))
  {
    wifiClient.println("<td><b>Switch 3</B><BR><a href=\"/switch3off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 3 is ON<BR><BR></td>");
  }
  else
  {
    wifiClient.println("<td><b>Switch 3</B><BR><a href=\"/switch3on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 3 is OFF<BR><BR></td>");
  }

  if (!digitalRead(Switch4))
  {
    wifiClient.println("<td><b>Switch 4</B><BR><a href=\"/switch4off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 4 is ON<BR><BR></td>");

  }
  else
  {
    wifiClient.println("<td><b>Switch 4</B><BR><a href=\"/switch4on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 4 is OFF<BR><BR></td>");
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

  int currentMonth = ptm->tm_mon + 1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  int currentYear = ptm->tm_year + 1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDateTime = String(currentYear) + ":" + String(currentMonth) + ":" + String(monthDay) + "T" + formattedTime;
  Serial.print("Current date: ");
  Serial.println(currentDateTime);
  return currentDateTime;

}
