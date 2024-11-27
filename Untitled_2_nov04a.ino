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
#include <Keypad.h>
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
Servo myservo;
char keys[ROWS][COLS] = {
  {'3', '2', '1', 'A'},
  {'#', '0', '*', 'B'},
  {'9', '8', '7', 'C'},
  {'6', '5', '4', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; // connect to the column pinouts of the keypad
int lcdColumns = 16;
int lcdRows = 2;
int buzz=18;
int servopin=23;
String setPassword="6754";
String enteredpass=" ";
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
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
  pinMode(18,OUTPUT);
  myservo.setPeriodHertz(50);
	myservo.attach(servopin, 500, 2400);
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  WiFi.begin("0165", "gaaagrrr");
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    delay(500);
  }
}
void loop() 
{
  //lcd.clear();
  ArduinoCloud.update();
  enter();
}
void enter()
{
  char key = getKey();
  if(key=='#')
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter password:");
    int i=0;
    while(true)
    {
      key = getKey();
      if(i==11)
      {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Max length");
          lcd.setCursor(0,1);
          lcd.print("Reached");
          delay(1000);
          break;
      }
      if(key!='*'||i!=0)
      {
          if(key=='*')
          {
              i--;
              enteredpass[i]='\0';
          }
          enteredpass[i]=key;
          i++;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter password:");
          lcd.setCursor(0, 1);
          lcd.print(enteredpass);
      }
      else if(key=='#')
      {
        k++;
        if(enteredpass.equals(setPassword))
        {
            digitalWrite(buzz,HIGH);
            delay(100);
            digitalWrite(buzz,LOW);
            myservo.write(180);
            delay(150);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Correct password");
            lcd.setCursor(0,1);
            lcd.print("Door opened");
            fb.pushString("Entries/Entry"+String(k)+"/Status","Opened");
            enteredpass="Correct Password";
            delay(500);
            myservo.write(90);
        }
        else
        {
            password="Wrong Password";
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Wrong password");
            lcd.setCursor(0,1);
            lcd.print("Door not opened");
            fb.pushString("Entries/Entry"+String(k)+"/Status","notOpened");
            myservo.write(90);
            delay(150);
        }
        fb.pushString("Entries/Entry"+String(k)+"/typed",enteredpass);
        fb.pushString("Entries/Entry"+String(k)+"/typedTime",String(printLocalTime()));
        lcd.clear();
        break;
      }
      else
      {
        break;
      }
    }
    lcd.clear();
  }
}
void onPasswordChange()  {
  k++;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Typed password:");
  lcd.setCursor(0,1);
  lcd.print(password);
  if(password.equals(setPassword))
  {
    digitalWrite(buzz,HIGH);
    delay(100);
    digitalWrite(buzz,LOW);
    myservo.write(180);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Correct Password");
    lcd.setCursor(0,1);
    lcd.print("Door Opened");
    delay(150);
    fb.pushString("Entries/Entry"+String(k)+"/Status","Opened");
    password="Correct Password \n Door Opened";
    delay(500);
    myservo.write(90);
  }
  else
  {
    password="Wrong Password \n Door not Opened";
    fb.pushString("Entries/Entry"+String(k)+"/Status","notOpened");
    myservo.write(90);
    delay(150);
  }
  fb.pushString("Entries/Entry"+String(k)+"/typed",password);
  fb.pushString("Entries/Entry"+String(k)+"/typedTime",String(printLocalTime()));
  lcd.clear();
}
char getKey() {
  for (byte c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {
        delay(50); // Debounce delay
        while (digitalRead(rowPins[r]) == LOW); // Wait for key release
        digitalWrite(colPins[c], HIGH);
        return keys[r][c];
      }
    }
    digitalWrite(colPins[c], HIGH);
  }
  return 0;
 }
