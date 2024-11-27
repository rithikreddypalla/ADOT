import firebase_admin
from firebase_admin import credentials, db
!pip install twilio
from twilio.rest import Client


# Find your Account SID and Auth Token at twilio.com/console
# and set the environment variables. See http://twil.io/secure

def alert():
    account_sid ="ACd0820c8fb546c38be1760536009548cc"
    auth_token ="a07946d79b75d7a24937ba7ffcf5f083"
    client = Client(account_sid, auth_token)

    call = client.calls.create(
                            twiml='<Response><Say>You are recieving this call because there has been a suspicious entry at the door please be alerted</Say></Response>',
                            to='+918247341184',
                            from_='+15856693126'
                        )
# Firebase credentials directly in the code (not recommended for production)
firebase_credentials = {
    "type": "service_account",
    "project_id": "adpsr-75e1f",
    "private_key_id": "4090e7b1b73e8f141c8df9fde567201ef7205121",
    "private_key": "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADAN...",
    "client_email": "firebase-adminsdk-7fyvc@adpsr-75e1f.iam.gserviceaccount.com",
    "client_id": "107099221824990410869",
    "auth_uri": "https://accounts.google.com/o/oauth2/auth",
    "token_uri": "https://oauth2.googleapis.com/token",
    "auth_provider_x509_cert_url": "https://www.googleapis.com/oauth2/v1/certs",
    "client_x509_cert_url": "https://www.googleapis.com/robot/v1/metadata/x509/firebase-adminsdk-7fyvc%40adpsr-75e1f.iam.gserviceaccount.com",
    "universe_domain": "googleapis.com"
}

# Initialize Firebase Admin SDK if not already initialized
if not firebase_admin._apps:
    cred = credentials.Certificate(firebase_credentials)
    firebase_admin.initialize_app(cred, {
        'databaseURL': 'https://adpsr-75e1f-default-rtdb.firebaseio.com/'
    })

# Function to fetch data from Firebase Realtime Database
def fetch_data_from_firebase(path):
    ref = db.reference(path)  # Specify the database path
    data = ref.get()
    return data

# Comparison function to calculate similarity
def compare(value):
    expected_password = "6754"
    count = 0
    # Convert strings to lists for modification
    expected_password_list = list(expected_password) 
    value_list = list(value)
    for i in range(0,len(expected_password),1):
        for j in range(0,len(value),1):
          if value_list[j] == expected_password_list[i]:
              count += 1
              # Modify the lists instead of the original strings
              expected_password_list[i]='&'  
              value_list[j]='.'
    return (count / len(value))

# Fetch specific data
status_data = fetch_data_from_firebase('Entries/Entry5/typed')

print("Status Data:")
for key, value in status_data.items():
    # if key == "typedpassword":
        print(f"  Key: {key}, Value: {value}")
        similarity_score = compare(value)
        print(f"Similarity Score: {similarity_score:.2f}")
        if(similarity_score <= 0.30):
          alert();
          break;
