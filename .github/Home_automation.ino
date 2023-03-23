#include<ESP8266WiFi.h>
#include "DHT.h"
#define DHTPIN D4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE); //calling an object dht from class DHT
#include "MQ135.h"

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial  // Comment this out to disable prints and save space
#include <CayenneMQTTESP8266.h>

// WiFi network info.
char ssid[] = "OP 8";
char wifiPassword[] = "1234567890";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "e61d7e00-5958-11ed-bf0a-bb4ba43bd3f6";
char password[] = "b23fb2d033e81e6090bbdff16a51f369b3a33cae";
char clientID[] = "8c89fd30-59f0-11ed-baf6-35fab7fd0ac8";

#define LightSwitch 0
#define ACSwitch 1
#define Temp 2

#define Humidity 3
#define Air_Quality 4
#define BrightnessControl 5
#define ACTempControl 6
#define FanSpeedControl 7
#define Fan 8
#define Fire 9
#define Rain 10
#define LIGHT_PIN D0 
#define AC_PIN D1
#define buzzer 10
#define motorPin1 D6
#define motorPin2 D7
#define SmokeSensor A0
#define enable D5
#define flameSensor D3
#define RainSensor D8

void setup()
{
	Serial.begin(9600);
	pinMode(LIGHT_PIN, OUTPUT);
  pinMode(AC_PIN, OUTPUT);
  initWiFi();
  Cayenne.begin(username, password, clientID);
  dht.begin();
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(SmokeSensor, INPUT);
  pinMode(RainSensor, INPUT);
  pinMode(flameSensor, INPUT);
  pinMode(enable, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void loop()
{
	Cayenne.loop();
}
void initWiFi(){
 WiFi.begin(ssid, wifiPassword);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WiFi.setAutoReconnect(true);  
}
// This function is called when data is sent from Cayenne.
CAYENNE_IN(LightSwitch)
{
	int value = getValue.asInt();
	// Write the value received to the digital pin.
  CAYENNE_LOG("Channel %d, pin %d, value %d", LightSwitch, LIGHT_PIN, value);
  digitalWrite(LIGHT_PIN,value);
}
CAYENNE_IN(ACSwitch)
{
  int value = getValue.asInt();
  // Write the value received to the digital pin.
  digitalWrite(AC_PIN, value);
  CAYENNE_LOG("Channel %d, pin %d, value %d", ACSwitch, AC_PIN, value);
}
CAYENNE_IN(Fan)
{
  int value = getValue.asInt();
  // Write the value received to the digital pin
  if (value==1){  
  digitalWrite(enable, 255);  
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
  CAYENNE_LOG("Send data for Fan Channel %d", value);
    }
  else{
  digitalWrite(enable, 0);  
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  CAYENNE_LOG("Send data for Fan Channel %d", value);
    }
  
}
CAYENNE_OUT(Air_Quality)
{
  int sensorValue = analogRead(SmokeSensor);
  MQ135 gasSensor = MQ135(SmokeSensor);
  float air_quality = (gasSensor.getPPM())/1000;      
  Serial.print("AirQuality=");
  Serial.print(air_quality,DEC);            
  Serial.println(" PPM");
  Cayenne.virtualWrite(Air_Quality, air_quality);
  CAYENNE_LOG("Channel %d, pin %d, value %d", Air_Quality, SmokeSensor, sensorValue);
  delay(100);
}
CAYENNE_OUT(Temp)
{
  float t = dht.readTemperature();
  Serial.print("temperature: ");
  Serial.println(t);
  //Write to Cayenne Dashboard
  Cayenne.virtualWrite(Temp, t, TYPE_TEMPERATURE, UNIT_CELSIUS);
  CAYENNE_LOG("Send data for Temperature Channel %d", Temp);
}
CAYENNE_OUT(Humidity){
  //Serial.println("humidity");
   float h = dht.readHumidity();;
    delay(1000);
  Serial.print("humidity: ");
  Serial.println(h);

  //Write to Cayenne Dashboard`
  Cayenne.virtualWrite(Humidity, h, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);
  CAYENNE_LOG("Send data for Humidity Channel %d", Humidity);
}
CAYENNE_OUT(Fire){
    int sensorValue=digitalRead(flameSensor);
    if (sensorValue)
    {
    Serial.println("Fire detected");
    tone(buzzer,2048);
    digitalWrite(buzzer,HIGH);
    delay(1000);
    Cayenne.virtualWrite(Fire, 1);
    CAYENNE_LOG("Send data for Fire Channel %d",1 );
    delay(1000);
    }
    else{
    digitalWrite(buzzer,LOW);
    Cayenne.virtualWrite(Fire, 0);
    CAYENNE_LOG("Send data for Fire Channel %d",0 );
    }
    delay(1000);
    
}
CAYENNE_OUT(Rain){
  int water=digitalRead(RainSensor);
  //Cayenne.virtualWrite(Rain, 1);
  if(water)
{
  Serial.println("Water leaking from the roof");
  tone(buzzer,2048);
  digitalWrite(buzzer,HIGH);
  delay(1000);
  Cayenne.virtualWrite(Rain, 1);
  CAYENNE_LOG("Send data for Rain Channel %d",1 );
}

else
{
  noTone(buzzer);
  digitalWrite(buzzer,LOW);
  Cayenne.virtualWrite(Rain, 0);
  CAYENNE_LOG("Send data for Rain Channel %d",0 );
}
}
void strt() {
  // Set motors to maximum speed

  // Turn on motor A & B
  digitalWrite(motorPin1, HIGH);
  digitalWrite(motorPin2, LOW);
}
void stp(){
  // Turn off motors
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
}
void setspeed(int i){
  analogWrite(enable,i);
}
