#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import argparse
from datetime import datetime

# --- Arguments CLI ---
parser = argparse.ArgumentParser(description="Monitor MQTT topic for test messages")
parser.add_argument("--broker", "-b", default="192.168.1.13", help="IP du broker MQTT")
parser.add_argument("--port", "-p", type=int, default=1884, help="Port MQTT")
parser.add_argument("--topic", "-t", default="Système d'Alerte pour la Sécurité d'un Véhicule", help="Topic MQTT")
args = parser.parse_args()

BROKER = args.broker
PORT = args.port
TOPIC = args.topic

def now():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def handle_payload(payload_str):
    payload = payload_str.strip()
    print(f"{now()} 📩 Message reçu : '{payload}'")

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

