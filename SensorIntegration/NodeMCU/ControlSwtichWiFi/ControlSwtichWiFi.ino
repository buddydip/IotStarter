#include <ESP8266WiFi.h>

const char* ssid = "MORPHEOUS";
const char* password = "whatever2020";

String controlState = "";

WiFiServer server(80);

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

}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  digitalWrite(3, HIGH);
  while (!client.available()) {
    delay(1);
    if (!client) {
    
      return;
    }
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Match the request


  if (request.indexOf("/light1on") > 0)  {
    digitalWrite(5, LOW);
    controlState ="{\"Switch1\":1}";

  }
  if (request.indexOf("/light1off") > 0)  {
    digitalWrite(5, HIGH);
    controlState ="{\"Switch1\":0}";

  }

  if (request.indexOf("/light2on") > 0)  {
    digitalWrite(4, LOW);
    controlState ="{\"Switch2\":1}";

  }
  if (request.indexOf("/light2off") > 0)  {
    digitalWrite(4, HIGH);
    controlState ="{\"Switch2\":0}";

  }
  if (request.indexOf("/light3on") > 0)  {
    digitalWrite(14, LOW);
    controlState ="{\"Switch3\":1}";

  }
  if (request.indexOf("/light3off") > 0)  {
    digitalWrite(14, HIGH);
    controlState ="{\"Switch3\":0}";

  }
  if (request.indexOf("/light4on") > 0)  {
    digitalWrite(12, LOW);
    controlState ="{\"Switch4\":1}";

  }
  if (request.indexOf("/light4off") > 0)  {
    digitalWrite(12, HIGH);
    controlState ="{\"Switch4\":0}";

  }

  if (request.indexOf("/light5on") > 0)  {
    digitalWrite(1, HIGH);
    controlState ="{\"Switch5\":1}";

  }
  if (request.indexOf("/light5off") > 0)  {
    digitalWrite(1, LOW);
    controlState ="{\"Switch5\":0}";

  }
  // Set ledPin according to the request
  //digitalWrite(ledPin, value);

  // Return the response
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
  client.println(".button {");
  client.println("background-color: #008CBA;");
  client.println("border: none;");
  client.println("color: white;");
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
  client.println("<br><br>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<b>Switch 1</B>");
  client.println("<a href=\"/light1on\"\"><button class=\"button\">Turn On </button></a>");
  client.println("<a href=\"/light1off\"\"><button class=\"button\">Turn Off </button></a><br />");
  client.println("</center>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<b>Switch 2</b>");
  client.println("<a href=\"/light2on\"\"><button class=\"button\">Turn On </button></a>");
  client.println("<a href=\"/light2off\"\"><button class=\"button\">Turn Off </button></a><br />");
  client.println("</center>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<b>Switch 3</b>");
  client.println("<a href=\"/light3on\"\"><button class=\"button\">Turn On </button></a>");
  client.println("<a href=\"/light3off\"\"><button class=\"button\">Turn Off </button></a><br />");
  client.println("</center>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<b>Switch 4</b>");
  client.println("<a href=\"/light4on\"\"><button class=\"button\">Turn On </button></a>");
  client.println("<a href=\"/light4off\"\"><button class=\"button\">Turn Off </button></a><br />");
  client.println("</center>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<b>Switch 5</b>");
  client.println("<a href=\"/light5on\"\"><button class=\"button\">Turn On </button></a>");
  client.println("<a href=\"/light5off\"\"><button class=\"button\">Turn Off </button></a><br />");
  client.println("</center>");
  client.println("<br><br>");
  client.println("<center>");
  client.println("<table border=\"5\">");
  client.println("<tr>");
  if (!digitalRead(5))
  {
    client.print("<td bgcolor='red'>Switch 1 is ON</td>");
 

  }
  else
  {
    client.print("<td bgcolor='green'>Switch 1 is OFF</td>");
    
  }

  client.println("<br />");

  if (!digitalRead(4))
  {
    client.print("<td bgcolor='red'>Switch 2 is ON</td>");
  
  }
  else
  {

    client.print("<td bgcolor='green'>Switch 2 is OFF</td>");
   
  }


  if (!digitalRead(14))

  {
    client.print("<td bgcolor='red'>Switch 3 is ON</td>");
    
  }

  else

  {
    client.print("<td bgcolor='green'>Switch 3 is OFF</td>");
   
  }


  if (!digitalRead(12))


  {


    client.print("<td bgcolor='red'>Switch 4 is ON</td>");
   
  }


  else


  {


    client.print("<td bgcolor='green'>Switch 4 is OFF</td>");
   

  }

   if (digitalRead(1))


  {


    client.print("<td bgcolor='red'>Switch 5 is ON</td>");
    
  }


  else


  {


    client.print("<td bgcolor='green'>Switch 5 is OFF</td>");
    

  }

  client.println("</tr>");


  client.println("</table>");

  client.println("<BR>");
  client.println(controlState);
  client.println("</center>");
  client.println("</html>");
  delay(1);
  Serial.println(controlState);
  Serial.println("Client disonnected");
  Serial.println("");

}
