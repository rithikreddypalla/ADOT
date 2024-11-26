/*
MOSI: 23-COPI-11
MISO: 19-CIPO-12
SCK: 18
SS: 5
*/
#include <ESP32Servo.h>
#include "thingProperties.h"
#include <WiFi.h>
#include <Firebase.h>
#include "time.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
Servo myservo;
int k=0;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
Firebase fb("https://adpsr-75e1f-default-rtdb.firebaseio.com/");//,"AIzaSyDdwRu2Z_KpWIS48iKbiza939SYDwYixeo");
String printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return " ";
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  return String(timeinfo.tm_hour)+":"+String(timeinfo.tm_min)+":"+String(timeinfo.tm_sec);
}
void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(1500); 
  lcd.init();
  lcd.backlight();
  myservo.setPeriodHertz(50);
	myservo.attach(13, 500, 2400);
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  WiFi.begin("ff", "12345678");
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }
}
void loop() {
  ArduinoCloud.update();
}
void onPasswordChange()  {
  k++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Typed password:");
  lcd.setCursor(0,1);
  lcd.print(password);
  if(password.equals("hihello"))
  {
    myservo.write(180);
    delay(150);
    fb.pushString("Entries/Entry"+String(k)+"/Status","Opened");
    password=" ";
    delay(500);
    myservo.write(90);
  }
  else
  {
    fb.pushString("Entries/Entry"+String(k)+"/Status","notOpened");
    myservo.write(90);
    delay(150);
  }
  fb.pushString("Entries/Entry"+String(k)+"/typed",password);
  fb.pushString("Entries/Entry"+String(k)+"/typedTime",String(printLocalTime()));
  lcd.clear();
}