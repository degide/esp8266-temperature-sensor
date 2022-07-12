
#include<Wire.h>
#include <ESP8266WiFi.h>
#include<LiquidCrystal_I2C.h>

int redLED = 14;
int greenLED = 16;
int buzzer = 12;
float temperature = 0;
float temperature_offset = 12.0083;
bool isUploading = false;

bool ON = true;
bool OFF = false;

WiFiClient wifiClient;
LiquidCrystal_I2C lcd(0x27, 16 , 2) ;

void setup() {
  Serial.begin(9600);
  WiFi.begin("dlink", "");
  lcd.begin();
  lcd.backlight();
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void switchAlert(bool turnOn){
  if(turnOn == true){
    digitalWrite(redLED, HIGH);
    digitalWrite(buzzer, HIGH);
    digitalWrite(greenLED, LOW);
    
  }else{
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
    digitalWrite(buzzer, LOW);
  }
}

void readTemp(){
  int rawData = analogRead(A0);
  float vcc = 5.0;
  float voltage = rawData * (vcc / 1024.0);
  temperature = voltage * 100;
  Serial.println("READING: "+ (String) temperature);
  temperature = temperature - temperature_offset;
}

void uploadTempReading(){
  if(isUploading == true) return;
  isUploading = true;
  String host = "192.168.1.150";
  int port = 80;
  String request = "POST /iot/?device=340722SPE0532022&temperature="+((String)temperature)+" HTTP/1.1";
  wifiClient.connect(host, port);
  wifiClient.println(request);
  wifiClient.println("Host: "+host);
  wifiClient.println("User-Agent: ESP8266 WeMos D1 Mini/1.0");
  wifiClient.println("C: ESP8266 WeMos D1 Mini/1.0");
  wifiClient.println();
  Serial.println("Upload Response: " + wifiClient.readStringUntil('\n'));
  isUploading = false;
}

void loop() {
  if(isUploading == true) return;
  
  readTemp();

  String tempStatus = "NORMAL";
  if(temperature > 35) tempStatus = "ABNORMAL";
  
  //Print to serial
  Serial.print("\n");
  Serial.println("Temp: "+ ((String) temperature) +" C");
  Serial.println("Status: "+ tempStatus);

  //Print on lcd
  lcd.clear();
  lcd.setCursor(0, 0); // col0, row0
  lcd.print("Temp: "+ ((String) temperature) +" C");
  lcd.setCursor(0, 1); // col0, row1
  lcd.print("Status: "+ tempStatus);
  
  if(temperature > 35){
    switchAlert(ON);
    uploadTempReading();
  }else{
    switchAlert(OFF);
  }
  
  delay(1000);
}
