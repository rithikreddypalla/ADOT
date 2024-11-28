#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <Firebase.h>
#include <time.h>
#include <ESP32Servo.h>
#include <Adafruit_Fingerprint.h>
//for fingerprint sensor
#define RX_PIN 16
#define TX_PIN 17
#define RELAY_PIN 14
#define LED_PIN 2
Adafruit_Fingerprint finger(&Serial2);
const int validFingerIDs[] = {1, 2};
const int validFingerCount = sizeof(validFingerIDs) / sizeof(validFingerIDs[0]);
//for servo
Servo myservo;
int servoPin=23;
//for keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {
  {'3', '2', '1', 'A'},
  {'#', '0', '*', 'B'},
  {'9', '8', '7', 'C'},
  {'6', '5', '4', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; // connect to the column pinouts of the keypad
//for lcd
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
//for buzzer
int buzz=18;
//password settings here
String pass="1234";
String enteredPass="";
//for firebase
Firebase fb("https://adpsr-75e1f-default-rtdb.firebaseio.com/");
//for entries
int k=0;
void setup() {
  Serial.begin(9600);
  WiFi.begin("0165", "gaaagrrr");
  int h=0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("-");
    h++;
    if(h==10)
    {
      break;
    }
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected to the Firebase");
  }
  else
  {
    Serial.println("Not connected to the Firebase");
  }
  lcd.init();
  lcd.backlight();
  myservo.setPeriodHertz(50);
	myservo.attach(servoPin, 500, 2400);
  lcd.clear();
  Serial2.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}
char getKey() 
{
  for (byte c = 0; c < COLS; c++) 
  {
    digitalWrite(colPins[c], LOW);
    for (byte r = 0; r < ROWS; r++) 
    {
      if (digitalRead(rowPins[r]) == LOW) 
      {
        delay(50); // Debounce delay
        while (digitalRead(rowPins[r]) == LOW);// Wait for key release
        digitalWrite(colPins[c], HIGH);
        Serial.println(keys[r][c]+"gvmjv");
        return keys[r][c];
      }
    }
    digitalWrite(colPins[c], HIGH);
  }
  return -1;
}
int getFingerprintID() 
{
  int result = finger.getImage();
  if (result != FINGERPRINT_OK) return -1;

  result = finger.image2Tz();
  if (result != FINGERPRINT_OK) return -1;

  result = finger.fingerFastSearch();
  if (result != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}
void loop()
{
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DOOR LOCKED");
  lcd.setCursor(0, 1);
  lcd.print("HELLO");
  enter();
  int fingerID = getFingerprintID();
  if (fingerID >=0)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Correct Finger");
    lcd.setCursor(0,1);
    lcd.print("Door opening");
    myservo.write(180);
    delay(5000);
    myservo.write(90);
  }
}
void enter()
{
  char getCh= getKey();
  if(getCh!=-1)
  {
    Serial.println("Enter kuagsddmA");
    if(getCh=='#')
    {
      lcd.clear();
      enteredPass="";
      lcd.setCursor(0,0);
      lcd.print("Enter Password:");
      int ch=0;
      while(1)
      {
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter Password:");
        lcd.setCursor(0,1);
        lcd.print(enteredPass);
        getCh= getKey();
        if(getCh=='#')
        {
          if(WiFi.status() == WL_CONNECTED)
          {

          }
          if(enteredPass.equals(pass))
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Password Matched");
            myservo.write(180);
            delay(5000);
            myservo.write(90);
          }
          else
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Wrong Password");
            lcd.setCursor(0,1);
            lcd.print("Door not opened");
          }
          break;
        }
        else if(getCh=='*'&&ch!=0)
        {
          enteredPass[ch-1]=' ';
          ch--;
        }
        else
        {
          enteredPass[ch]=getCh;
          ch++;
        }
      }
    }
  }
}
/*
else if(fingerID == -1)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Put Finger");
    lcd.setCursor(0,1);
    lcd.print("Properly");
  }
  else if(fingerID == -2)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Wrong Finger");
    lcd.setCursor(0,1);
    lcd.print("Door not opened");
  }
  */
