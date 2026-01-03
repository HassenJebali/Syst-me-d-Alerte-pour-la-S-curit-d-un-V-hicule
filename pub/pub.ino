#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_bt.h"   // 🔴 Désactiver Bluetooth

// Informations WiFi
const char* ssid = "TT_ABC0_2.4G";
const char* password = "R4r3dTANe9";

// Broker MQTT
const char* mqtt_server = "192.168.1.13";
const int mqtt_port = 1884;
const char* topic = "systeme_alerte_vehicule";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(100);

  // 🔴 Désactivation Bluetooth (important)
  btStop();

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.println();
  Serial.print("Connexion à ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connecté");
    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Échec connexion WiFi");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (client.connect("ESP32_Publisher")) {
      Serial.println("Connecté !");
    } else {
      Serial.print("Échec, rc=");
      Serial.print(client.state());
      Serial.println(" → nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Message de test
  const char* message = "Alerte : intrusion détectée !";

  Serial.print("Publication : ");
  Serial.println(message);

  client.publish(topic, message);
  delay(5000);  // toutes les 5 secondes
}
