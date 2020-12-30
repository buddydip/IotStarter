/*Gas sensor example*/
int mq3 = A0; //connect Gas sensor to analogic pin A0
int buzzer = 7; //connect buzzer to digital pin 7
int red_led = 8;  //connect red led to digital pin 8 
int green_led = 9;   //connect green led to digital pin 9
int threshold =300; //change the threshold value for your use

void setup(){
   Serial.begin(9600); //initialize serial comunication at 9600 bps
   pinMode(red_led, OUTPUT);//define red_led as output
   pinMode(green_led, OUTPUT); //define green_led as output
   pinMode(buzzer, OUTPUT); //define buzzer as output
   digitalWrite(red_led, LOW);//red_led off
   digitalWrite(green_led, LOW);//green_led off
}
 
void loop()
{
    int mq3 = analogRead(mq3);//read sensor value
    Serial.println(mq3);//print on serial monitor the sensor value
    if(mq3>=threshold){//check if it is exceeded the threshold value
        digitalWrite(red_led,HIGH);//red_led on
        digitalWrite(green_led,LOW);//green_led off
        tone(buzzer,200);//play buzzer
        Serial.println(" Attention threhold exceeded!!!");//print on serial monitor the overcoming threshold notice
        delay(500);
    }
    else{
        digitalWrite(red_led,LOW);//red_led off
        digitalWrite(green_led,HIGH);//green_red on
    }
    delay(100); //slow down the output
}
