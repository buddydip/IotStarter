/*
Web switch for 8 Channel Relay
Uses MQTT messages for switch on/off
Also uses webserver on ESP32 for direct switch control
*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid     = "MORPHEOUS";
const char* password = "whatever2020";
const char* nodeID = "POWERCONTROL3";

//MQTT Connection
const char* mqtt_server = "Smarty";
const int mqtt_port = 1883;
const char* mqtt_topic = "smarty/switchcontrol/mazstudy";

//JSON message for switch control
String controlState = "";
StaticJsonDocument<200> doc;


//PIN Switch Mapping
const int Switch1 = 12;
const int Switch2 = 15;
const int Switch3 = 18;
const int Switch4 = 21;
const int Switch5 = 22;
const int Switch6 = 23;
const int Switch7 = 26;
const int Switch8 = 27;

String switch1name = String(nodeID) + ".Switch1";
String switch2name = String(nodeID) + ".Switch2";
String switch3name = String(nodeID) + ".Switch3";
String switch4name = String(nodeID) + ".Switch4";
String switch5name = String(nodeID) + ".Switch5";
String switch6name = String(nodeID) + ".Switch6";
String switch7name = String(nodeID) + ".Switch7";
String switch8name = String(nodeID) + ".Switch8";

//Wifiserver instance at port 80
WiFiServer server(80);
WiFiUDP ntpUDP;
const int utcOffsetInSeconds = 19800;

NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//MQTT client at port 1883
WiFiClient wifimqClient;
PubSubClient mqttclient;


//callback function for MQTT message
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

  if (switchID.equals(switch5name) && (digitalRead(Switch5) == switchState))
  {
    digitalWrite(Switch5, !switchState);
  }

  if (switchID.equals(switch6name) && (digitalRead(Switch6) == switchState))
  {
    digitalWrite(Switch6, !switchState);
  }

  if (switchID.equals(switch7name) && (digitalRead(Switch7) == switchState))
  {
    digitalWrite(Switch7, !switchState);
  }

  if (switchID.equals(switch8name) && (digitalRead(Switch8) == switchState))
  {
    digitalWrite(Switch8, !switchState);
  }

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
  }
  else
  {
    Serial.print("Failed to connect to MQTT broker with state ");
    Serial.println(mqttclient.state());
    delay(2000);
  }
  return mqttclient.state();
}


void sendMQTTMessage(String switchName, int switchState)
{

    doc.clear();
    controlState="";
    doc["SwitchID"] = switchName;
    doc["SwitchState"] = switchState;
    serializeJson(doc, controlState);
    Serial.println(controlState);

    mqttclient.publish(mqtt_topic, controlState.c_str());
    doc.clear();
}


void setup()
{

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


  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "<hostname>.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin(nodeID)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  timeClient.begin();


  //connect to MQTT broker
  connectMQTT();
}

int value = 0;

void loop() {

  //check MQTT Connection
  if (mqttclient.state() != 0)
  {
    connectMQTT();
  }

  mqttclient.loop();
  timeClient.update();

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,

        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            // Display the Page
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println(""); //  do not forget this one
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<meta name='apple-mobile-web-app-capable' content='yes' />");
            client.println("<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />");
            client.println("</head>");
            client.println("<body bgcolor = \"#f7e6ec\">");
            client.println("<style>");
            client.println(".redbutton {");
            client.println("background-color: #ff0000;");
            client.println("border: 2px solid black;");
            client.println("color: white;");
            client.println("padding: 15px 32px;");
            client.println("text-align: center;");
            client.println("text-decoration: none;");
            client.println("display: inline-block;");
            client.println("font-size: 16px;");
            client.println("margin: 4px 2px;");
            client.println("cursor: pointer;");
            client.println("}");

            client.println(".greenbutton {");
            client.println("background-color: #00ff75;");
            client.println("border: 2px solid black;");
            client.println("color: black;");
            client.println("padding: 15px 32px;");
            client.println("text-align: center;");
            client.println("text-decoration: none;");
            client.println("display: inline-block;");
            client.println("font-size: 16px;");
            client.println("margin: 4px 2px;");
            client.println("cursor: pointer;");
            client.println("}");

            client.println("</style>");
            client.println("<hr/><hr>");
            client.println("<h4><center> Smart Switch Control </center></h4>");
            client.println("<hr/><hr>");
            client.println("<center>");
            client.println("<table border=\"1\" width=\"100%\">");
            client.println("<tr>");


            if (!digitalRead(Switch1))
            {
              client.println("<td><b>Switch 1</B><BR><a href=\"/switch1off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 1 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 1</B><BR><a href=\"/switch1on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 1 is OFF<BR><BR></td>");
            }

            if (!digitalRead(Switch2))
            {
              client.println("<td><b>Switch 2</B><BR><a href=\"/switch2off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 2 is ON<BR><BR></td>");
            }
            else
            {

              client.println("<td><b>Switch 2</B><BR><a href=\"/switch2on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 2 is OFF<BR><BR></td>");
            }


            if (!digitalRead(Switch3))
            {
              client.println("<td><b>Switch 3</B><BR><a href=\"/switch3off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 3 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 3</B><BR><a href=\"/switch3on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 3 is OFF<BR><BR></td>");
            }

            if (!digitalRead(Switch4))
            {
              client.println("<td><b>Switch 4</B><BR><a href=\"/switch4off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 4 is ON<BR><BR></td>");

            }
            else
            {
              client.println("<td><b>Switch 4</B><BR><a href=\"/switch4on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 4 is OFF<BR><BR></td>");
            }

            client.println("</tr><tr>");

            if (!digitalRead(Switch5))
            {
              client.println("<td><b>Switch 5</B><BR><a href=\"/switch5off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 5 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 5</B><BR><a href=\"/switch5on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 5 is OFF<BR><BR></td>");
            }

            if (!digitalRead(Switch6))
            {
              client.println("<td><b>Switch 6</B><BR><a href=\"/switch6off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 6 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 6</B><BR><a href=\"/switch6on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 6 is OFF<BR><BR></td>");
            }

            if (!digitalRead(Switch7))
            {
              client.println("<td><b>Switch 7</B><BR><a href=\"/switch7off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 7 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 7</B><BR><a href=\"/switch7on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 7 is OFF<BR><BR></td>");
            }


            if (!digitalRead(Switch8))
            {
              client.println("<td><b>Switch 8</B><BR><a href=\"/switch8off\"\"><button class=\"redbutton\">Turn Off</button></a><br/>Switch 8 is ON<BR><BR></td>");
            }
            else
            {
              client.println("<td><b>Switch 8</B><BR><a href=\"/switch8on\"\"><button class=\"greenbutton\">Turn On</button></a><br/>Switch 8 is OFF<BR><BR></td>");
            }


            client.println("</tr>");
            client.println("</table>");
            client.println("<BR>");
            client.println("</center>");
            client.println("</html>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see the client request":
        if (currentLine.endsWith("GET /switch1on")) {
          digitalWrite(Switch1, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch1name, 1);
        }
        if (currentLine.endsWith("GET /switch1off")) {
          digitalWrite(Switch1, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch1name, 0);
        }

        // Check to see the client request":
        if (currentLine.endsWith("GET /switch2on")) {
          digitalWrite(Switch2, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch2name, 1);
        }
        if (currentLine.endsWith("GET /switch2off")) {
          digitalWrite(Switch2, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch2name, 0);
        }

        // Check to see the client request":
        if (currentLine.endsWith("GET /switch3on")) {
          digitalWrite(Switch3, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch3name, 1);
        }
        if (currentLine.endsWith("GET /switch3off")) {
          digitalWrite(Switch3, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch3name, 0);
        }
        // Check to see the client request":
        if (currentLine.endsWith("GET /switch4on")) {
          digitalWrite(Switch4, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch4name, 1);
        }
        if (currentLine.endsWith("GET /switch4off")) {
          digitalWrite(Switch4, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch4name, 0);
        }
        // Check to see the client request":
        if (currentLine.endsWith("GET /switch5on")) {
          digitalWrite(Switch5, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch5name, 1);
        }
        if (currentLine.endsWith("GET /switch5off")) {
          digitalWrite(Switch5, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch5name, 0);
        }
        // Check to see the client request":
        if (currentLine.endsWith("GET /switch6on")) {
          digitalWrite(Switch6, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch6name, 1);
        }
        if (currentLine.endsWith("GET /switch6off")) {
          digitalWrite(Switch6, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch6name, 0);
        }
        // Check to see the client request":
        if (currentLine.endsWith("GET /switch7on")) {
          digitalWrite(Switch7, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch7name, 1);
        }
        if (currentLine.endsWith("GET /switch7off")) {
          digitalWrite(Switch7, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch7name, 0);
        }    
        // Check to see the client request":
        if (currentLine.endsWith("GET /switch8on")) {
          digitalWrite(Switch8, LOW);               // GET /H turns the LED on
          sendMQTTMessage(switch8name, 1);
        }
        if (currentLine.endsWith("GET /switch8off")) {
          digitalWrite(Switch8, HIGH);                // GET /L turns the LED off
          sendMQTTMessage(switch8name, 0);
        }            
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
