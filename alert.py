"""
ADOT Alert System
-----------------
Monitors Firebase for suspicious door entries and sends alerts via Twilio.
Refactored for readability and security. Store credentials in environment variables or a config file for production use.
"""

import os
import time
import firebase_admin
from firebase_admin import credentials, db
from twilio.rest import Client

# Load credentials from environment variables or a secure config file
FIREBASE_CRED_PATH = os.getenv('FIREBASE_CRED_PATH', 'firebase_credentials.json')
TWILIO_SID = os.getenv('TWILIO_SID', 'YOUR_TWILIO_SID')
TWILIO_TOKEN = os.getenv('TWILIO_TOKEN', 'YOUR_TWILIO_TOKEN')
ALERT_TO = os.getenv('ALERT_TO', '+911234567890')
ALERT_FROM = os.getenv('ALERT_FROM', '+15005550006')

# Initialize Firebase Admin SDK
if not firebase_admin._apps:
    cred = credentials.Certificate(FIREBASE_CRED_PATH)
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://adpsr-75e1f-default-rtdb.firebaseio.com/'
    })

# Initialize Twilio Client
client = Client(TWILIO_SID, TWILIO_TOKEN)

# List of valid passwords (should be securely managed in production)
listp = ["1234"]

def send_alert():
    """Send an alert call via Twilio."""
    call = client.calls.create(
        twiml='<Response><Say>You are receiving this call because there has been a suspicious entry at the door. Please be alerted.</Say></Response>',
        to=ALERT_TO,
        from_=ALERT_FROM
    )
    print("Alert sent!")

def fetch_data_from_firebase(path):
    """Fetch data from Firebase Realtime Database at the given path."""
    ref = db.reference(path)
    return ref.get()

def compare(value):
    """Compare input value to known passwords and return max similarity (0-1)."""
    maxsim = 0
    if not value:
        return 0
    for expected_password in listp:
        count = 0
        expected_password_list = list(expected_password)
        value_list = list(value)
        for i in range(len(expected_password_list)):
            for j in range(len(value_list)):
                if value_list[j] == expected_password_list[i]:
                    count += 1
                    expected_password_list[i] = '&'
                    value_list[j] = '.'
                    break
        similarity = count / len(value)
        if similarity > maxsim:
            maxsim = similarity
    return maxsim

def monitor_entries():
    """Continuously monitor entries and trigger alert if suspicious."""
    k = 0
    while True:
        try:
            path1 = f'Entries/Entry{k}/typed'
            status1 = fetch_data_from_firebase(path1)
            l = 1
            while True:
                try:
                    path2 = f'Passwords/pass{l}'
                    status2 = fetch_data_from_firebase(path2)
                    if status2:
                        for key, value in status2.items():
                            if len(listp) > l:
                                listp[l] = value
                            else:
                                listp.append(value)
                        l += 1
                    else:
                        break
                except Exception as e:
                    print(f"Error fetching passwords: {e}. Retrying...")
                    time.sleep(5)

            if status1:
                for key, value in status1.items():
                    similarity_score = compare(value)
                    print(f"Entry {k} - Key: {key}, Value: {value}, Similarity: {similarity_score:.2f}")
                    if similarity_score <= 0.30:
                        send_alert()
                k += 1
            else:
                print(f"No data for Entry {k}. Retrying...")
                time.sleep(2)
        except Exception as e:
            print(f"Error: {e}. Retrying...")
            time.sleep(5)

if __name__ == "__main__":
    print("Starting ADOT Alert Monitor...")
    monitor_entries()
