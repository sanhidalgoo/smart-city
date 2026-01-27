from flask import Flask, render_template
import firebase_admin
from firebase_admin import credentials, db as firebase_db

app = Flask(__name__)

# Initialize Firebase
cred = credentials.Certificate("./service_account.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://test2-a0f86-default-rtdb.firebaseio.com/'
})
db = firebase_db.reference()

@app.route('/')
def index():
    # Get data from Firebase
    data = db.child("arduino_data").get()

    records = []
    if data:
        for key, value in data.items():
            record = {
                'id': key,
                'timestamp': value.get('input', {}).get('timestamp', 'N/A'),
                'co2': value.get('input', {}).get('co2', 'N/A'),
                'critical_traffic1': value.get('input', {}).get('critical_traffic1', False),
                'critical_traffic2': value.get('input', {}).get('critical_traffic2', False),
                'isLate': value.get('input', {}).get('isLate', False),
                'reasoning': value.get('output', {}).get('reasoning', 'N/A'),
                'green_time1': value.get('output', {}).get('timings', {}).get('GREEN_TIME1', 'N/A'),
                'green_time2': value.get('output', {}).get('timings', {}).get('GREEN_TIME2', 'N/A'),
                'yellow_time1': value.get('output', {}).get('timings', {}).get('YELLOW_TIME1', 'N/A'),
                'yellow_time2': value.get('output', {}).get('timings', {}).get('YELLOW_TIME2', 'N/A'),
            }
            records.append(record)

    # Sort by timestamp descending (newest first)
    records.sort(key=lambda x: x['timestamp'], reverse=True)

    return render_template('index.html', records=records)

if __name__ == '__main__':
    app.run(debug=True, port=5000)
