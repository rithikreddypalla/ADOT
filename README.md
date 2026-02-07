# ADOT: Advanced Door Operation & Tracking

ADOT is a smart door security system that combines hardware (ESP32/Arduino, keypad, fingerprint sensor, LCD) and software (Python, Firebase, Twilio) to provide secure, monitored access and real-time alerts for suspicious entries.

## Features
- **Multi-factor authentication:** Password and fingerprint-based access
- **Cloud integration:** Uses Firebase Realtime Database for logging entries and password management
- **Remote alerts:** Sends automated phone call alerts via Twilio if a suspicious entry is detected
- **LCD feedback:** User-friendly interface for status and instructions
- **Password management:** Change and update passwords from the device

## Hardware Components
- ESP32 or compatible microcontroller
- 4x4 Keypad
- Fingerprint sensor (Adafruit-compatible)
- LCD (I2C 16x2)
- Servo motor (for lock)
- WiFi connectivity

## Software Components
- Arduino sketch (`arduino_code.ino`): Handles hardware logic, user input, and Firebase communication
- Python script (`alert.py`): Monitors Firebase for suspicious entries and triggers Twilio alerts
- `thingProperties.h`: Arduino IoT Cloud integration (optional)

## Setup
1. **Hardware:** Assemble the components as per the pin configuration in the Arduino sketch.
2. **Firebase:**
	- Create a Firebase project and Realtime Database
	- Download the service account key as `firebase_credentials.json`
3. **Twilio:**
	- Create a Twilio account and get your Account SID, Auth Token, and phone numbers
	- Set these as environment variables for the Python script
4. **Python dependencies:**
	- Install with `pip install firebase-admin twilio`
5. **Run:**
	- Upload the Arduino sketch to your ESP32
	- Run the Python script: `python alert.py`

## Usage
- Enter the password and/or scan your fingerprint to unlock the door
- All entries are logged to Firebase
- If an entry is deemed suspicious (low similarity to valid passwords), an alert call is sent

## Security Notes
- **Do not** hardcode credentials in your code. Use environment variables or config files (not committed to version control)
- Change default passwords and keep your Firebase and Twilio credentials secure

## Author
Rithik (2026)

---
*This project is a demonstration of IoT, cloud, and security integration for smart home applications.*