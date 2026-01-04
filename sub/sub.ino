#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Informations WiFi
const char* ssid = "PC_DE_king";
const char* password = "000000001";

// Broker MQTT
const char* mqtt_server = "192.168.137.1"; 
const int mqtt_port = 1884;
const char* topic = "systeme_alerte_vehicule";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== LED d’alerte =====
#define LED_ALERT 25   
#define BUZZER 26

void setup_wifi() {
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("IP : ");
  Serial.println(WiFi.localIP());
}

// ===== CALLBACK MQTT =====
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("\n📩 Message MQTT reçu");

  // Convert payload -> String
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  // ===== JSON Parsing =====
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("❌ Erreur JSON : ");
    Serial.println(error.c_str());
    return;
  }

  // ===== Lecture des données =====
  float ax = doc["ax"];
  float ay = doc["ay"];
  float az = doc["az"];
  float acc = doc["acceleration"];
  float angle = doc["ang"];

  bool choc = doc["choc"];
  bool inclinaison = doc["inclinaison"];
  bool dark = doc["dark"];
  bool obstacle = doc["obstacle"];

  float distance = doc["distance_cm"];
  float temp = doc["temp"];
  float pressure = doc["pre"];

  // ===== Affichage structuré =====
  Serial.println("------ Données Véhicule ------");
  Serial.printf("Acc XYZ : %.2f | %.2f | %.2f\n", ax, ay, az);
  Serial.printf("Acc totale : %.2f g\n", acc);
  Serial.printf("Angle : %.1f °\n", angle);
  Serial.printf("Température : %.1f °C\n", temp);
  Serial.printf("Pression : %.1f hPa\n", pressure);
  Serial.printf("Distance obstacle : %.1f cm\n", distance);

  Serial.printf("Choc : %s\n", choc ? "OUI" : "NON");
  Serial.printf("Inclinaison : %s\n", inclinaison ? "OUI" : "NON");
  Serial.printf("Obstacle : %s\n", obstacle ? "OUI" : "NON");
  Serial.printf("Sombre : %s\n", dark ? "OUI" : "NON");

  // ===== Gestion des alertes =====
  if (choc || obstacle || inclinaison) {
    for(int i=0; i<5; i++){   // clignote 5 fois
        digitalWrite(LED_ALERT, HIGH);
        digitalWrite(BUZZER, HIGH);
        delay(200);
        digitalWrite(LED_ALERT, LOW);
        digitalWrite(BUZZER, LOW);
        delay(200);
    }
} 

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (client.connect("ESP32_Subscriber")) {
      Serial.println("Connecté !");
      client.subscribe(topic);
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" Nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_ALERT, OUTPUT);
  pinMode(BUZZER, OUTPUT); 

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
