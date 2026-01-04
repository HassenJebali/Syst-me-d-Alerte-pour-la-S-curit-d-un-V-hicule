#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import argparse
import firebase_admin
from firebase_admin import credentials, db
import json
from datetime import datetime


# --- Initialiser Firebase ---
cred = credentials.Certificate("/root/mqtt-project-db78c-firebase-adminsdk-fbsvc-97741b1516.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://mqtt-project-db78c-default-rtdb.firebaseio.com/'
})

# --- Arguments CLI ---
parser = argparse.ArgumentParser(description="Monitor MQTT topic for test messages")
parser.add_argument("--broker", "-b", default="127.0.0.1", help="IP du broker MQTT")
parser.add_argument("--port", "-p", type=int, default=1884, help="Port MQTT")
parser.add_argument("--topic", "-t", default="systeme_alerte_vehicule", help="Topic MQTT")
args = parser.parse_args()

BROKER = args.broker
PORT = args.port
TOPIC = args.topic

def now():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def handle_payload(payload_str):
    payload = payload_str.strip()
    print(f"{now()} 📩 Message reçu : '{payload}'")

    try:
        data = json.loads(payload)  # tenter de convertir en JSON
    except Exception:
        data = {"message": payload}  # sinon stocke texte brut

    # Créer une référence dans Firebase par topic
    ref = db.reference(f'mqtt_data/{TOPIC.replace("/", "_")}')
    ref.push({
        'timestamp': now(),
        'data': data
    })
    print(f"{now()} ✅ Données envoyées à Firebase")


# --- Callback MQTT quand message reçu ---
def on_message(client, userdata, msg):
    try:
        payload_str = msg.payload.decode('utf-8', errors='replace')
    except Exception:
        payload_str = str(msg.payload)
    handle_payload(payload_str)

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"{now()} ✅ Connecté au broker {BROKER}:{PORT} — souscription à '{TOPIC}'")
        client.subscribe(TOPIC)
    else:
        print(f"{now()} ❌ Échec connexion (code {rc})")

def on_disconnect(client, userdata, rc):
    print(f"{now()} ⚠️ Déconnecté (code {rc}). Tentative de reconnexion automatique...")

# --- Setup client ---
client = mqtt.Client()
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

print(f"{now()} ➤ Connexion au broker {BROKER}:{PORT} (topic: {TOPIC})")
client.connect(BROKER, PORT, keepalive=60)
client.loop_forever()