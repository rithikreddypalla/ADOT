import firebase_admin
from firebase_admin import credentials, db
# !pip install twilio
from twilio.rest import Client
import time

# Firebase credentials (use environment variables or secure file storage in production)
firebase_credentials = {
    "type": "service_account",
    "project_id": "adpsr-75e1f",
    "private_key_id": "YOUR_PRIVATE_KEY_ID",
    "private_key": "-----BEGIN PRIVATE KEY-----\nYOUR_PRIVATE_KEY\n-----END PRIVATE KEY-----\n",
    "client_email": "firebase-adminsdk-7fyvc@adpsr-75e1f.iam.gserviceaccount.com",
    "client_id": "107099221824990410869",
    "auth_uri": "https://accounts.google.com/o/oauth2/auth",
    "token_uri": "https://oauth2.googleapis.com/token",
    "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
    "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-7fyvc%40adpsr-75e1f.iam.gserviceaccount.com",
    "universe_domain": "googleapis.com"
}

# Twilio credentials
account_sid = "ACd0820c8fb546c38be1760536009548cc"
auth_token = "a07946d79b75d7a24937ba7ffcf5f083"

# Initialize Firebase Admin SDK
if not firebase_admin._apps:
    cred = credentials.Certificate(firebase_credentials)
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://adpsr-75e1f-default-rtdb.firebaseio.com/'
    })

# Initialize Twilio Client
client = Client(account_sid, auth_token)
listp = ["1234"]
# Function to send an alert via Twilio
def alert():
    call = client.calls.create(
        twiml='<Response><Say>You are receiving this call because there has been a suspicious entry at the door. Please be alerted.</Say></Response>',
        to='+918247341184',
        from_='+15856693126'
    )
    print("Alert sent!")

# Fetch data from Firebase Realtime Database
def fetch_data_from_firebase(path):
    ref = db.reference(path)  # Specify the database path
    data = ref.get()
    return data

# Comparison function to calculate similarity
def compare(value):
    maxsim = 0
    if len(value) == 0:
        return 0
    for expected_password in listp:
        print(expected_password)
        count = 0
        expected_password_list = list(expected_password)
        value_list = list(value)
        for i in range(len(expected_password_list)):
            for j in range(len(value_list)):
                if value_list[j] == expected_password_list[i]:
                    count += 1
                    expected_password_list[i] = '&'
                    value_list[j] = '.'
                    break  # Stop searching for this character in value_list
        similarity = count / len(value)
        if similarity > maxsim:
            maxsim = similarity
    return maxsim

# Infinite monitoring loop
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
                    print(f"Updating passwords from pass{l}...")
                    for key, value in status2.items():
                        print(f"Key: {key}, Value: {value}")
                        if len(listp) > l:
                            print("chec")
                            listp[l] = value
                        else:
                            print("did")
                            listp.append(value)
                    l += 1
                else:
                    print(l,"done")
                    break
            except Exception as e:
                print(f"Error fetching passwords: {e}. Retrying...")
                time.sleep(5)

        if status1:
            print(f"Processing Entry {k}...")
            for key, value in status1.items():
                print(f"Key: {key}, Value: {value}")
                similarity_score = compare(value)
                print(f"Similarity Score: {similarity_score:.2f}")
                if similarity_score <= 0.30:
                    alert()
            k += 1  # Move to the next entry
        else:
            print(f"No data for Entry {k}. Retrying...")
            time.sleep(2)
    except Exception as e:
        print(f"Error: {e}. Retrying...")
        time.sleep(5)
