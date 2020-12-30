

int Led = 13 ;// define LED Interface
int Shock = 10; // define the vibration sensor interface
int Shock1 = A2; // define the vibration sensor interface
int val; // define numeric variables val
int val1;
void setup ()
{
  pinMode (Led, OUTPUT) ; // define LED as output interface
  pinMode (Shock, INPUT) ; // output interface defines vibration sensor
  Serial.begin(9600);
}
void loop ()
{
  val = digitalRead (Shock) ; // read digital interface is assigned a value of 3 val
  val1 = analogRead (Shock1) ; // read digital interface is assigned a value of 3 val
  Serial.println(val);
  //Serial.println(val1);
  if (val == HIGH) // When the shock sensor detects a signal, LED flashes
  {
    digitalWrite (Led, HIGH);
  }
  else
  {
    digitalWrite (Led, LOW);
  }
}


