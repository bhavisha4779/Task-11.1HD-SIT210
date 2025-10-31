from flask import Flask, render_template_string, jsonify
import paho.mqtt.client as mqtt
from geopy.distance import geodesic
from gpiozero import Buzzer
import threading, time

app = Flask(__name__)
mqtt_broker = "localhost"
mqtt_topic = "accidents/alerts/bike_001"

buzzer = Buzzer(18)

HOSPITALS = {
    "City Hospital Sunam": (30.06, 75.77),
    "PGI Chandigarh": (30.73, 76.79),
    "Neelam Hospital Rajpura": (30.52, 76.66)
}

accident_data = {"latitude": 0, "longitude": 0, "hospital": "N/A", "distance": 0}

def find_nearest_hospital(lat, lon):
    acc = (lat, lon)
    nearest, dist = None, float('inf')
    for name, coords in HOSPITALS.items():
        d = geodesic(acc, coords).km
        if d < dist:
            nearest, dist = name, d
    return nearest, round(dist, 2)

def buzz_for(sec=3):
    buzzer.on()
    time.sleep(sec)
    buzzer.off()

def on_message(client, userdata, msg):
    import json
    data = json.loads(msg.payload.decode())
    lat, lon = data.get("latitude"), data.get("longitude")
    hospital, distance = find_nearest_hospital(lat, lon)
    accident_data.update({
        "latitude": lat,
        "longitude": lon,
        "hospital": hospital,
        "distance": distance
    })
    threading.Thread(target=buzz_for, args=(2,)).start()
    print(f"Accident at ({lat}, {lon}) -> {hospital} ({distance} km)")

client = mqtt.Client()
client.on_message = on_message
client.connect(mqtt_broker)
client.subscribe(mqtt_topic)

threading.Thread(target=client.loop_forever, daemon=True).start()

HTML = """
<!DOCTYPE html>
<html>
<head>
<title>Smart Accident Detection Dashboard</title>
<style>
body{font-family:Poppins;background:#0b132b;color:white;text-align:center}
.card{background:#1c2541;padding:20px;margin:60px auto;border-radius:15px;width:70%;box-shadow:0 0 25px rgba(0,0,0,0.4)}
h1{color:#5bc0be}
.value{font-size:20px;margin:10px}
button{background:#5bc0be;border:none;padding:8px 16px;color:white;border-radius:6px;cursor:pointer}
</style>
<script>
async function refreshData(){
 let r=await fetch('/data');let d=await r.json();
 document.getElementById('lat').innerText=d.latitude.toFixed(5);
 document.getElementById('lon').innerText=d.longitude.toFixed(5);
 document.getElementById('hos').innerText=d.hospital;
 document.getElementById('dist').innerText=d.distance+' km';
}
setInterval(refreshData,2000);
</script>
</head>
<body onload="refreshData()">
<h1>Smart Accident Detection System</h1>
<div class="card">
  <p class="value">Latitude: <span id="lat">0</span></p>
  <p class="value">Longitude: <span id="lon">0</span></p>
  <p class="value">Nearest Hospital: <span id="hos">N/A</span></p>
  <p class="value">Distance: <span id="dist">0 km</span></p>
</div>
</body>
</html>
"""

@app.route('/')
def home(): return render_template_string(HTML)

@app.route('/data')
def data(): return jsonify(accident_data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
