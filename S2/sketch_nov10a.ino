#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

// Wi-Fi
const char* SSID = "FIESC_IOT_EDU";
const char* WIFI_PASS = "8120gv08";

// HiveMQ Cloud
const char* MQTT_SERVER = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "Placa_2";
const char* MQTT_PASS = "Placa123";

// Tópicos MQTT
const char* TOPICO_SENSOR = "sensor/distancia";
const char* TOPICO_LED = "sensor/comando";

// Sensor ultrassônico
const int trigPin = 9;
const int echoPin = 10;
long duracao;
float distancia;
bool detectou = false;

#define PINO_LED LED_BUILTIN

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Recebido no tópico ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mensagem);

  if (mensagem.equalsIgnoreCase("acender")) {
    digitalWrite(PINO_LED, HIGH);
  } else if (mensagem.equalsIgnoreCase("apagar")) {
    digitalWrite(PINO_LED, LOW);
  }
}

void setup() {
  pinMode(PINO_LED, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);

  // Conecta ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi...");
  WiFi.begin(SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" conectado!");

  // Configura MQTT
  client.setInsecure(); // sem certificado
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  Serial.print("Conectando ao HiveMQ...");
  while (!mqtt.connected()) {
    String clientId = "ESP32-S1-";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }

  mqtt.subscribe(TOPICO_LED);
  Serial.println("Assinado no tópico de LED.");
}

void loop() {
  // Reconecta se necessário
  if (!mqtt.connected()) {
    while (!mqtt.connected()) {
      String clientId = "ESP32-S1-";
      clientId += String(random(0xffff), HEX);
      mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS);
      delay(500);
    }
    mqtt.subscribe(TOPICO_LED);
  }

  // Leitura do sensor ultrassônico
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracao = pulseIn(echoPin, HIGH);
  distancia = duracao * 0.0343 / 2; // converte para cm

  if (distancia > 0 && distancia < 10 && !detectou) {
    detectou = true;
    digitalWrite(PINO_LED, HIGH);
    mqtt.publish(TOPICO_SENSOR, "Objeto detectado!");
    Serial.println("Objeto detectado!");
  } else if (distancia >= 10 && detectou) {
    detectou = false;
    digitalWrite(PINO_LED, LOW);
    mqtt.publish(TOPICO_SENSOR, "Área livre");
    Serial.println("Área livre");
  }

  mqtt.loop();
  delay(300);
}