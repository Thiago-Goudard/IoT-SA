#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Servo.h>

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const char* mqtt_host = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const uint16_t mqtt_port = 883;
const char* mqtt_client_id = "Node_S3";
const char* MQTT_USER = "SEU_USUARIO";     
const char* MQTT_PASS = "SUA_SENHA";       

WiFiClientSecure secureClient;
PubSubClient client(secureClient);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_PIN 2
#define SERVO1_PIN 12
#define SERVO2_PIN 13

Servo servo1;
Servo servo2;

void setup_wifi() {
  WiFi.begin(SSID.c_str(), PASS.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    bool ok;
    if (MQTT_USER[0] == '\0') {
      ok = client.connect(mqtt_client_id);
    } else {
      ok = client.connect(mqtt_client_id, MQTT_USER, MQTT_PASS);
    }
    if (ok) {
      client.subscribe("S3/Servo1");
      client.subscribe("S3/Servo2");
    } else {
      delay(3000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  if (String(topic) == "S3/Servo1") {
    int angle = message.toInt();
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    servo1.write(angle);
  } else if (String(topic) == "S3/Servo2") {
    int angle = message.toInt();
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    servo2.write(angle);
  }
}

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  if (duration == 0) return 999.0;
  float distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  setup_wifi();

  secureClient.setInsecure(); // remove se for usar CA válido
  client.setServer(mqtt_host, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float distancia = medirDistancia();

  if (distancia < 10.0) {
    digitalWrite(LED_PIN, HIGH);
    client.publish("S3/Presenca", "1");
  } else {
    digitalWrite(LED_PIN, LOW);
    client.publish("S3/Presenca", "0");
  }

  delay(1500);
}

