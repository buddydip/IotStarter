#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>

//Wi-fi connection
const char *ssid = "MORPHEOUS";
const char *password = "whatever2020";
const char *nodeID = "AMBIENCESENSOR1";


//MQTT Connection
const char *mqtt_server = "Smarty.local";
const int mqtt_port = 1883;
const char *mqtt_topic = "smarty/ambience";

//JSON message for switch control
StaticJsonDocument<200> doc;

//Wifiserver instance at port 80
WiFiServer server(80);

//MQTT client at port 1883
WiFiClient wifimqClient;
PubSubClient mqttclient;
WiFiUDP ntpUDP;
const int utcOffsetInSeconds = 19800;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


//define PINS
SoftwareSerial s(12,14);

String outputString;


void setup() {
  s.begin(9600);
  Serial.begin(9600);
  Serial.flush();

  timeClient.begin();

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
    mqttclient.publish("debugsmarty", msg.c_str());

  }
  else
  {
    Serial.print("Failed to connect to MQTT broker with state ");
    Serial.println(mqttclient.state());
    String msg = String("Not Connected : " + mqttclient.state());
    mqttclient.publish("debugsmarty", msg.c_str());
    delay(2000);
  }
  return mqttclient.state();
}

void MQTTcallback(char* topic, byte* payload, unsigned int length)
{
  /*
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


    mqttclient.publish("debugsmarty", ("Switch :"+switchID+"  State  :"+switchState+"  Time : "+stateTime).c_str());
  */
}

void loop() {

  MDNS.update();

  //check MQTT Connection
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();
  timeClient.update();
Serial.println(s.available());
  if (s.available()>0)
  {
    outputString =s.read();
    Serial.print("Read from Serial : ");
    Serial.println(outputString);
  }
/*
  if (outputString != "")
  {
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, outputString);
   
    // Test if parsing succeeds.
    if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
    }
    
    doc["Time"] = getCurrentDateTime();
    outputString="";
    serializeJson(doc, outputString);

    Serial.println(outputString);
    mqttclient.publish(mqtt_topic, outputString.c_str());
    doc.clear();
  }
  */
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
  wifiClient.println("<h4><center> Ambience Monitor </center></h4>");
  wifiClient.println("<hr/><hr>");
  wifiClient.println("<center>");
  wifiClient.println("<table border=\"5\">");

  wifiClient.println("<tr>");
  wifiClient.print("<td>" + String(outputString) + "</td>");
  wifiClient.println("</tr>");


  wifiClient.println("</table>");
  wifiClient.println("<BR>");
  wifiClient.println("</center>");
  wifiClient.println("</html>");

  Serial.println("Client disonnected");
  Serial.println("");

}


String getCurrentDateTime()
{
  unsigned long epochTime = timeClient.getEpochTime();

  String formattedTime = timeClient.getFormattedTime();

  int currentHour = timeClient.getHours();

  int currentMinute = timeClient.getMinutes();

  int currentSecond = timeClient.getSeconds();

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;

  int currentMonth = ptm->tm_mon + 1;

  int currentYear = ptm->tm_year + 1900;

  //Print complete date:
  String currentDateTime = String(currentYear) + ":" + String(currentMonth) + ":" + String(monthDay) + "T" + formattedTime;

  return currentDateTime;

}
