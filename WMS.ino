#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"
//BMP280
Adafruit_BMP280 bmp;
//SCL-A5 & SDA-A4
float pressure=0.0;

//DHT Sensor
#define DHTPIN 4 
#define DTYPE DHT22
DHT dht(DHTPIN, DTYPE);
float temp;
float humi;
//END DHT Sensor


//Dust Sensor
#define mPin A0
#define lPin 5
short sampTime=280;
short dTime=40;
short sTime=9680;
float vMeasure=0;
//float calcVolt=0;
//float dustDensity=0;

//End Dust Sensor

//MQ Series Sensor
#define mQa A1
short mQaVal;
#define mQb A2
short mQbVal;


//Rain Sensor
#define rain 7
short rainVal;


//Uv Sensor
#define UVOUT A3
short uvLevel;
#define refLevel 670

int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}
 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//End Uv Sensor


#define RX 2
#define TX 3
#define dht_apin A0 // Analog Pin sensor is connected to
String SSID = "Titanic~wifi";       //WiFi Name
String PASS = "58354047"; // WiFi PASSWORD
String API = "928WXYDRDK0DMJ70";   // ThingSpeak Key
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
  
SoftwareSerial esp8266(RX,TX); 
  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ SSID +"\",\""+ PASS +"\"",20,"OK");
  
  dht.begin();
  pinMode(lPin, OUTPUT);
  pinMode(mQa, INPUT);
  pinMode(mQb, INPUT);
  pinMode(rain, INPUT);
  pinMode(UVOUT, INPUT); 
  Serial.println("ML8511 example");
  Serial.println(F("BMP280 sensor test"));
  
  if (!bmp.begin(0x76)) { // The default I2C address is 0x76, if it doesn't work, try 0x77
  Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  
//Over Sampling BMP280
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                  Adafruit_BMP280::SAMPLING_X2,     
                  Adafruit_BMP280::SAMPLING_X16,  
                  Adafruit_BMP280::FILTER_X16,    
                  Adafruit_BMP280::STANDBY_MS_500);

}

void loop() {
  
 String getData = "GET /update?api_key="+ API +"&field1="+getTemperatureValue()+"&field2="+getHumidityValue()
                   +"&field3="+getDustValue()+"&field4="+getMQaValue()+"&field5="+getMQbValue()
                   +"&field6="+getuvValue()+"&field7="+getbmpValue()+"&field8="+getrainValue();
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
 esp8266.println(getData);
 delay(1000); 
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
  
}


String getTemperatureValue(){
  temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println("Temp Failed...");
    return;
  }
  return String(temp);  
}

String getHumidityValue(){
  humi = dht.readHumidity();  
  if (isnan(humi)) {
    Serial.println("Humidity Failed...");
    return;
  }
   return String(humi); 
}

String getDustValue(){
  
  digitalWrite(lPin, LOW);
  delayMicroseconds(sampTime);
  vMeasure= analogRead(mPin);
  delayMicroseconds(dTime);
  digitalWrite(lPin, HIGH);
  delayMicroseconds(sTime);

  //calcVolt = vMeasure *(5.0/ 1024.0);
  //dustDensity= 170 * calcVolt - 0.1; 
  Serial.print(vMeasure);
  
  return String(vMeasure);
  }

 String getMQaValue(){
     mQaVal= analogRead(mQa);
     delay(500);
     Serial.print(mQaVal);    

     return String(mQaVal);
    }

 String getMQbValue(){
     mQbVal= analogRead(mQb);
     delay(500);
     Serial.print(mQbVal);  
     return String(mQbVal);  
    }

 String getrainValue(){
     rainVal= analogRead(rain);
     delay(500);
     Serial.print(rainVal);  
     return String(rainVal);  
    }

 String getuvValue(){
     uvLevel = averageAnalogRead(UVOUT);
     float outputVoltage = 3.3 / refLevel * uvLevel;
     float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0);
     Serial.print(uvLevel);
     Serial.print(outputVoltage);
     Serial.print(uvIntensity);

     return String(uvLevel);
 }


 String getbmpValue(){
  
  //float temperature = bmp.readTemperature();
  //float altitude = bmp.readAltitude(1013.25);
  pressure = bmp.readPressure() / 100.0F;
  Serial.print(pressure);
  
  return String(pressure);
  }

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("Ok");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
