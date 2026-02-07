bool isUnlocked = false;
bool disTyped = false;

#include <Keypad.h>
#include <WiFi.h>
#include <Firebase.h>
#include <time.h>
#include <ESP32Servo.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

// --- Hardware and Library Setup ---
// Fingerprint sensor pins
#define RX_PIN 16
#define TX_PIN 17
Adafruit_Fingerprint finger(&Serial2);
int validFingerCount = 6; // Number of valid fingerprints enrolled

// Firebase setup
Firebase fb("https://adpsr-75e1f-default-rtdb.firebaseio.com/");

// LCD setup
int lcdColumns = 16;
int lcdRows = 2;
int z = 0;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
int k = 0; // Entry counter for Firebase
int pcn = 1; // Password change counter
int flag = 0; // Door status flag
Servo myservo;
char keys[ROWS][COLS] = {
  {'1', '2', '3', ' '},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'F'},
  {'*', '0', '#', ' '}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

// Password and input buffers
char password[17] = "1234";
char input[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
int fingerID = 0;
byte inputIndex = 0;
int attempt = 0;
bool isUnlocked = false;
bool disTyped = false;
const int servoPin = 18;

// --- Setup Function ---
void setup() {
  Serial.begin(9600); // Start serial communication
  // Connect to WiFi for Firebase
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
  // Servo setup
  myservo.attach(servoPin);
  myservo.write(90); // Initial position (locked)

  Serial.println("Enter Password:");

  // Initialize keypad pins
  for (byte i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  for (byte i = 0; i < COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH); 
  }
  Serial2.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN); // Fingerprint sensor
  pinMode(2, OUTPUT); // Door lock relay/indicator
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  delay(1000);
  lcd.clear();
}


// --- Main Loop ---
void loop() 
{
  // Allow up to 5 attempts for password/fingerprint
  if (attempt <= 4) {
    z = enterPassword(0); // Prompt for password
    fingerID = getFingerprintID(); // Check fingerprint
    if (fingerID >= 0 && isValidFingerID(fingerID)) {
      // Fingerprint recognized, unlock door
      lcd.clear();
      Serial.println("Correct Finger");
      lcd.setCursor(0,0);
      lcd.print("Correct Finger");
      lcd.setCursor(0,1);
      lcd.print("Door opening");
      digitalWrite(2, HIGH);
      Serial.println("Door opening...");
      myservo.write(180); // Open lock
      delay(1000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Welcome");
      lcd.setCursor(0,1);
      lcd.print("Door closing");
      myservo.write(90);  // Close lock
      digitalWrite(2, LOW);
      Serial.println("Door Closed");
      delay(500);
    } else if (fingerID >= 0) {
      // Fingerprint not recognized
      Serial.println("Fingerprint not recognized.");
    }
  } else {
    // Too many failed attempts
    Serial.println("Too many attempts. Please wait...");
    lcd.setCursor(0,0);
    lcd.print("attempts exceed");
    delay(5000);  // Wait before allowing new attempts
    attempt = 0;  // Reset counter
  }
}

// --- Password Entry and Management ---
// g: 0 = normal entry, 1 = change password, 2 = confirm new password
int enterPassword(int g) 
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Password:");
  Serial.println(inputIndex+"-"+input[inputIndex]);
  // Display asterisks for entered characters
  for(int vik=0; vik<inputIndex-1; vik++) {
    lcd.setCursor(vik,1);
    lcd.print("*");
  }
  if(inputIndex!=0) {
    lcd.setCursor(inputIndex-1,1);
    lcd.print(input[inputIndex-1]);
  }
  delay(200);
  char key = getKey();
  if (key!='K') {
    if (key == '*') {
      // End of password entry
      input[inputIndex] = '\0';
      if(g==2) {
        // Confirm new password
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Password changed");
        strcpy(password,input);
        inputIndex =0;
        for(int vik=0;vik<17;vik++) input[vik]='\0';
        int rssi = -30; // WiFi.RSSI();
        Serial.print("Signal Strength (RSSI): ");
        Serial.print(rssi);
        Serial.println(" dBm"); 
        if(WiFi.status()==WL_CONNECTED&&(rssi>=-30)) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Updating Data");
          lcd.setCursor(0,1);
          lcd.print("to Firebase...");
          fb.pushString("Passwords/pass"+String(pcn),String(input));
          pcn++;
        }
        return 20;
      }
      if (strcmp(input, password) == 0) {
        if(g==0) {
          // Password correct, unlock
          Serial.println("Password correct. Unlocking...");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Password Matched");
          isUnlocked = true;
          attempt=0;
          digitalWrite(2,HIGH);
          myservo.write(180); // Open lock
          delay(1000);
          Serial.println("lock opening");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Welcome");
          flag=1;
          delay(2000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Door closing");
          myservo.write(90);
          delay(250);  // Close lock
          digitalWrite(2,LOW);
          lcd.clear();
        } else if(g==1) {
          // Start password change
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Enter New");
          lcd.setCursor(0,1);
          lcd.print("Password");
          delay(1000);
          inputIndex =0;
          for(int vik=0;vik<17;vik++) input[vik]='\0';
          return 8;
        }
      } else {
        // Wrong password
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Wrong Password");
        g=0;
        flag=0;
        lcd.setCursor(0,1);
        lcd.print("Door not opened");
        Serial.println("Incorrect password.");
        attempt++;
        delay(1000);
        lcd.clear();
      }
      int rssi = -30; // WiFi.RSSI();
      Serial.print("Signal Strength (RSSI): ");
      Serial.print(rssi);
      Serial.println(" dBm"); 
      if(WiFi.status()==WL_CONNECTED&&(rssi>=-30)) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Updating Data");
        lcd.setCursor(0,1);
        lcd.print("to Firebase...");
        fb.pushString("Entries/Entry"+String(k)+"/typed",input);
        fb.pushString("Entries/Entry"+String(k)+"/status",flag==1?"Opened":"NotOpened");
        k++;
      }
      inputIndex = 0;
    } else if (key == 'B' && inputIndex==0) {
      // Start password change
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter password:");
      lcd.setCursor(0,1);
      lcd.print("First");
      delay(1000);
      changePassword();
    } else if(key=='B' && inputIndex>0) {
      // Backspace
      input[inputIndex--]='\0';
    } else if(key=='F' && g==0) {
      // Fingerprint enrollment
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Firstly Verify");
      lcd.setCursor(0,1);
      lcd.print("Fingerprint");
      delay(1000);
      for(int vik=0;vik<=10;vik++) {
        fingerID = getFingerprintID();
        if (fingerID >= 0 && isValidFingerID(fingerID)) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Fingerprint");
          lcd.setCursor(0,1);
          lcd.print("Validated");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Place new");
          lcd.setCursor(0,1);
          lcd.print("Finger");
          int git=0;
          while(true) {
            git++;
            if(enrollFingerprint(validFingerCount+1)) {
              break;
            }
            if(git>2) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enrollment");
              lcd.setCursor(0,1);
              lcd.print("Failed");
              break;
            }
          }
          return 11;
        }
      }
    } else if (attempt != 5 && key>='0' && key<='9') {
      // Add digit to input
      if (inputIndex <= 16) {          
        input[inputIndex] = key;
        inputIndex++;
        Serial.print("Input: ");
        Serial.println(input[inputIndex - 1]);
      } else {
        Serial.println("Input too long, resetting.");
        lcd.setCursor(0,0);
        lcd.print("Maxlen 16 only");
        delay(1000);
        lcd.print("Enter Password:");
      }
    }
    delay(250);
  }
  return 9;
}
// --- Password Change Handler ---
void changePassword()
{
  inputIndex=0;
  while(true) {
    if(enterPassword(1)==8) {
      while(true) {
        if(enterPassword(2)==20) {
          break;
        }
      }
      break;
    }
  }
}
// --- Fingerprint Enrollment ---
bool enrollFingerprint(int id) {
  int p;
  Serial.println("Waiting for valid finger...");
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOFINGER) continue;
    else return false;
  }
  if (finger.image2Tz(1) != FINGERPRINT_OK) return false;
  Serial.println("Remove finger...");
  delay(2000);
  Serial.println("Place the same finger again...");
  while ((p = finger.getImage()) != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOFINGER) continue;
    else return false;
  }
  if (finger.image2Tz(2) != FINGERPRINT_OK) return false;
  if (finger.createModel() != FINGERPRINT_OK) return false;
  if (finger.storeModel(id) != FINGERPRINT_OK) return false;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enrollment");
  lcd.setCursor(0,1);
  lcd.print("Successfull");
  validFingerCount=validFingerCount+1;
  delay(1000);
  return true;
}

// --- Keypad Input Handler ---
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
  return 'K';
}

// --- Fingerprint ID Handler ---
int getFingerprintID() {
  int result = finger.getImage();
  if (result == FINGERPRINT_NOFINGER) {
    // No finger detected
    return -2; 
  } else if (result != FINGERPRINT_OK) {
    Serial.println("Error capturing fingerprint image.");
    return -1;
  }
  result = finger.image2Tz();
  if (result != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Put Properly");
    delay(300);
    Serial.println("Error converting fingerprint image to template.");
    delay(500);
    return -1; 
  }
  result = finger.fingerFastSearch();
  if (result == FINGERPRINT_NOTFOUND) {
    lcd.clear();
    lcd.print("Not valid finger");
    delay(300);
    Serial.println("No valid fingerprint found.");
    delay(500);
    return -3; 
  } else if (result != FINGERPRINT_OK) {
    Serial.println("Error during fingerprint search.");
    return -1; 
  }
  // Found a match
  return finger.fingerID;
}


// --- Fingerprint Validity Checker ---
bool isValidFingerID(int fingerID) {
  if (validFingerCount >= fingerID) {
    return true;
  }
  return false;
}
