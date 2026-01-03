#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include "esp_bt.h" 

// Informations WiFi
const char* ssid = "PC_DE_king";
const char* password = "000000001";

// Broker MQTT
const char* mqtt_server = "192.168.1.13";
const int mqtt_port = 1884;
const char* topic = "systeme_alerte_vehicule";

// ===== Capteurs =====
MPU9250_asukiaaa imu;
Adafruit_BMP280 bmp;

// ===== Seuils =====
const float SHOCK_THRESHOLD = 2.5;      // g
const float TILT_THRESHOLD  = 30.0;     // degrés

WiFiClient espClient;
PubSubClient client(espClient);



void setup_wifi() {
  delay(100);

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

  Wire.begin(21, 22);

  imu.setWire(&Wire);
  imu.beginAccel();
  imu.beginGyro();
  imu.beginMag();

  if (!bmp.begin(0x76)) {
    Serial.println("❌ BMP280 non détecté");
  } else {
    Serial.println("✅ BMP280 OK");
  }
}


void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  imu.accelUpdate();

  float ax = imu.accelX();
  float ay = imu.accelY();
  float az = imu.accelZ();

  // Accélération totale
  float acc_total = sqrt(ax * ax + ay * ay + az * az);

  // Inclinaison (axe X)
  float angle = atan2(ax, az) * 180.0 / PI;

  bool choc = acc_total > SHOCK_THRESHOLD;
  bool inclinaison = abs(angle) > TILT_THRESHOLD;

  float temperature = bmp.readTemperature();
  float pressure = bmp.readPressure() / 100.0;

  // ===== JSON MQTT =====
  String payload = "{";
  payload += "\"ax\":" + String(ax, 2) + ",";
  payload += "\"ay\":" + String(ay, 2) + ",";
  payload += "\"az\":" + String(az, 2) + ",";
  payload += "\"acc_total\":" + String(acc_total, 2) + ",";
  payload += "\"angle\":" + String(angle, 1) + ",";
  payload += "\"choc\":" + String(choc ? "true" : "false") + ",";
  payload += "\"inclinaison\":" + String(inclinaison ? "true" : "false") + ",";
  payload += "\"temp\":" + String(temperature, 1) + ",";
  payload += "\"pression\":" + String(pressure, 1);
  payload += "}";

  Serial.println(payload);
  client.publish(topic, payload.c_str());

  delay(1000);
}
