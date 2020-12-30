int irsensor1 = 6;
int irsensor2 = A2;
int led1 = 13;

void setup() 
{
  Serial.begin(9600);  
  pinMode(irsensor1 , INPUT);
  pinMode(irsensor2 , INPUT);
  pinMode(led1, OUTPUT);
}

void loop() {

  int readsen = digitalRead(irsensor1);
  int readsen1 = analogRead(irsensor2);
  Serial.println  (readsen);
  Serial.println  (readsen1);
  if( readsen == HIGH )
  {
    digitalWrite(led1, HIGH); 
  }
  else
  {
    digitalWrite(led1, !HIGH); 
  }

  delay(100);
}


