#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//constants
#define VIN 3.3 // V power voltage, 3.3v in case of NodeMCU
#define R 10 // Voltage divider resistor value


//Wi-fi connection
const char *ssid = "MORPHEOUS";
const char *password = "whatever2020";
const char *nodeID = "STUDYAMBIENCE";


//MQTT Connection
const char *mqtt_server = "Smarty.local";
const int mqtt_port = 1883;
const char *mqtt_topic = "smarty/ambience/mainstudy";

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
#define PIR_PIN 12
#define LDR_PIN A0


String outputString = "";
int BOARD_RESOLUTION = 1024 ; // The analogic board resolution, for example NodeMCU ESP8266 is 10 bit (from 0 to 1023)
long lastMotionTime = 0;
long lastLightTime = 0;
String lastMotionTimeformatted = "";
String lastLightTimeformatted = "";
long detectionInterval = 5000;
float light = 0;
float illumLux = 0;


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

void setup() {

  Serial.begin(115200);

  timeClient.begin();

  //connect to Wifi
  connectWifi();

  //enable OTA
  enableOTA();


  //connect to MQTT broker
  connectMQTT();

  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

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
  }
  else
  {
    Serial.print("Failed to connect to MQTT broker with state ");
    Serial.println(mqttclient.state());
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

  */
}

int conversion(int raw_val)
{
  // Conversion rule
  float Vout = (raw_val * (VIN / 1023));// Conversion analog to voltage
  float RLDR = (R * (VIN - Vout))/Vout; // Conversion voltage to resistance
  int lux = 500/(RLDR/1000); // Conversion resitance to lumen
  if(lux < 0)
  {
    lux = 0;
  }  
  return lux;
}

void loop() {

  if ((WiFi.status() != WL_CONNECTED))
  {
    connectWifi();
  }

  MDNS.update();

  //check MQTT Connection
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();
  timeClient.update();

  ArduinoOTA.handle();


  Serial.print("motion Pin : ");
  Serial.println(digitalRead(PIR_PIN));

  long nowTime = millis();

  if (digitalRead(PIR_PIN) == HIGH)
  {

    Serial.println(lastMotionTimeformatted);

    Serial.println("Motion Detected!");
    if ((nowTime - lastMotionTime) > detectionInterval)
    {
      lastMotionTimeformatted = getCurrentDateTime();
      outputString = "";
      doc["ParamName"] = "Motion";
      doc["ParamValue"] = true;
      doc["Time"] = lastMotionTimeformatted;
      serializeJson(doc, outputString);
      Serial.println(outputString);
      lastMotionTime = nowTime;
      mqttclient.publish(mqtt_topic, outputString.c_str());
      doc.clear();
    }
  }

  light = analogRead(LDR_PIN);
  illumLux = conversion(light);
  Serial.print("light Pin : ");
  Serial.println(String(light));

  Serial.print("Illuminance : ");
  Serial.println(String(illumLux));


  if (illumLux < BOARD_RESOLUTION)
  {

    Serial.println("Darkness Detected!");
  }
  else
  {
    
    Serial.println("Light Detected!");
  }

   
    Serial.print("Actual Interval :");
    Serial.println(nowTime - lastLightTime);
    Serial.print("Fixed Interval :");
    Serial.println(detectionInterval);

    
    if ((nowTime - lastLightTime) > detectionInterval)
    {
      lastLightTimeformatted = getCurrentDateTime();
      outputString = "";
      doc["ParamName"] = "Light";
      doc["ParamValue"] = illumLux;
      doc["Time"] = nowTime;
      serializeJson(doc, outputString);
      Serial.println(outputString);
      lastLightTime = nowTime;
      mqttclient.publish(mqtt_topic, outputString.c_str());
      doc.clear();
    }
 

  // Check if a client has connected
  WiFiClient wifiClient = server.available();
  while (!wifiClient.available()) {
    delay(1);
    if (!wifiClient) {
      delay(1000);
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
  wifiClient.println("<h4><center> Ambience Monitor " + String(nodeID) + "</center></h4>");
  wifiClient.println("<hr/><hr>");
  wifiClient.println("<center>");
  wifiClient.println("<table border=\"5\">");

  wifiClient.println("<tr>");
  wifiClient.print("<td>Last Motion Time</td>");
  wifiClient.print("<td>" + lastMotionTimeformatted + "</td>");
  wifiClient.println("</tr>");


  wifiClient.println("<tr>");
  wifiClient.print("<td>Light Intensity</td>");
  wifiClient.print("<td>" + String(illumLux > BOARD_RESOLUTION?"HIGH":"LOW") +"  "+illumLux + " Lum</td>");
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

  //  int monthDay = ptm->tm_mday;
  //
  //  int currentMonth = ptm->tm_mon + 1;
  //
  //  int currentYear = ptm->tm_year + 1900;

  //Print complete date:
  //  String currentDateTime = String(currentYear) + ":" + String(currentMonth) + ":" + String(monthDay) + "T" + formattedTime;

  return formattedTime;

}
