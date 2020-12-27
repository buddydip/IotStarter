#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//Wi-fi connection
const char* ssid = "MORPHEOUS";
const char* password = "whatever2020";

//MQTT Connection
const char* mqtt_server = "192.168.1.8";
const int mqtt_port = 1883;


//JSON message for switch control
String controlState = "";

//Wifiserver instance at port 80
WiFiServer server(80);

//MQTT client at port 1883
WiFiClient wifiClient;
PubSubClient mqttclient(wifiClient);

void setup() {
  Serial.begin(9600);
  delay(10);
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
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  //connect to MQTT server
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(MQTTcallback);

  if (mqttclient.connect("ESP8266"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(mqttclient.state());
      delay(2000);
    }
  mqttclient.subscribe("test");
}


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
  Serial.print(message);
  if (message == "on") 
  {
    digitalWrite(1, HIGH);
  }
  else if (message == "off") 
  {
    digitalWrite(1, LOW);
  }
  Serial.println();
  Serial.println("-----------------------");
}

void loop() {
  // Check if a client has connected
  wifiClient = server.available();
  if (!wifiClient) {
    
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  digitalWrite(1, HIGH);
  while (!wifiClient.available()) {
    delay(1);
    if (!wifiClient) {
    digitalWrite(1, LOW);
      return;
    }
  }

  // Read the first line of the request
  String request = wifiClient.readStringUntil('\r');
  Serial.println(request);
  wifiClient.flush();

  // Match the request


  if (request.indexOf("/light1on") > 0)  {
    digitalWrite(5, LOW);
    controlState ="{\"Switch1\":1}";
    mqttclient.publish("test", "{\"Switch1\":1}");

  }
  if (request.indexOf("/light1off") > 0)  {
    digitalWrite(5, HIGH);
    controlState ="{\"Switch1\":0}";
    mqttclient.publish("test", "{\"Switch1\":0}");
  }

  if (request.indexOf("/light2on") > 0)  {
    digitalWrite(4, LOW);
    controlState ="{\"Switch2\":1}";
    mqttclient.publish("test", "{\"Switch2\":1}");

  }
  if (request.indexOf("/light2off") > 0)  {
    digitalWrite(4, HIGH);
    controlState ="{\"Switch2\":0}";
    mqttclient.publish("test", "{\"Switch2\":0}");

  }
  if (request.indexOf("/light3on") > 0)  {
    digitalWrite(14, LOW);
    controlState ="{\"Switch3\":1}";
    mqttclient.publish("test", "{\"Switch3\":1}");

  }
  if (request.indexOf("/light3off") > 0)  {
    digitalWrite(14, HIGH);
    controlState ="{\"Switch3\":0}";
    mqttclient.publish("test", "{\"Switch3\":0}");

  }
  if (request.indexOf("/light4on") > 0)  {
    digitalWrite(12, LOW);
    controlState ="{\"Switch4\":1}";
    mqttclient.publish("test", "{\"Switch4\":1}");

  }
  if (request.indexOf("/light4off") > 0)  {
    digitalWrite(12, HIGH);
    controlState ="{\"Switch4\":0}";
    mqttclient.publish("test", "{\"Switch4\":0}");

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
  wifiClient.println(controlState);
  wifiClient.println("</center>");
  wifiClient.println("</html>");
  delay(1);
  Serial.println(controlState);
  Serial.println("Client disonnected");
  Serial.println("");

}
