#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <ESP32Servo.h>

#define PINO_LED 2
#define TRIG 26
#define ECHO 25
#define PINO_SERVO1 19
#define PINO_SERVO2 18
#define PINO_PRESENCA 14

WiFiClientSecure client;
PubSubClient mqtt(client);

Servo servo3;
Servo servo4;

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* BROKER_URL  = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int   BROKER_PORT = 8883;
const char* BROKER_USER = "Placa_3";
const char* BROKER_PASS = "Placa123";

const char* TOPIC_PUBLISH_PRESENCA = "Projeto S3 Presenca3";
const char* TOPIC_PUBLISH_OBJETO   = "Projeto S3 Ultrassom3";

const char* TOPICO_SUBSCRIBE = "S1 iluminacao";

const char* TOPIC_PUBLISH_1 = "Projeto S2 Distancia1";
const char* TOPIC_PUBLISH_2 = "Projeto S2 Distancia2";

unsigned long lastPublish = 0;
int publishInterval = 3000;

long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);
  long distancia = (duracao * 0.034) / 2;

  return distancia;
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  if (mensagem == "acender") {
    digitalWrite(PINO_LED, HIGH);
  } 
  else if (mensagem == "apagar") {
    digitalWrite(PINO_LED, LOW);
  } 
  else if (String(topic) == TOPIC_PUBLISH_1) {
    if (mensagem == "objeto_proximo") {
      servo3.write(90);
    } else if (mensagem == "objeto_longe") {
      servo3.write(45);
    }
  } 
  else if (String(topic) == TOPIC_PUBLISH_2) {
    if (mensagem == "objeto_proximo") {
      servo4.write(90);
    } else if (mensagem == "objeto_longe") {
      servo4.write(45);
    }
  }

  Serial.println(mensagem);
}

void conectarWiFi() {
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
  }
}

void conectarMQTT() {
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  client.setInsecure();
  mqtt.setCallback(callback);

  while (!mqtt.connected()) {
    String clientId = "S3_" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      mqtt.subscribe(TOPICO_SUBSCRIBE);
      mqtt.subscribe(TOPIC_PUBLISH_1);
      mqtt.subscribe(TOPIC_PUBLISH_2);
      mqtt.subscribe("Projeto/S3/Controle");
    } 
    else {
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PINO_LED, OUTPUT);
  pinMode(PINO_PRESENCA, INPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  servo3.attach(PINO_SERVO1);
  servo4.attach(PINO_SERVO2);
  servo3.write(0);
  servo4.write(0);

  conectarWiFi();
  conectarMQTT();
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();
  mqtt.loop();

  long distancia = medirDistancia(TRIG, ECHO);
  Serial.println(distancia);

  if (distancia > 0 && distancia < 10) {
    mqtt.publish(TOPIC_PUBLISH_1, "objeto_proximo");
  } 
  else if (distancia > 10) {
    mqtt.publish(TOPIC_PUBLISH_2, "objeto_longe");
  }

  unsigned long agora = millis();
  if (agora - lastPublish >= publishInterval) {
    lastPublish = agora;
    int presenca = digitalRead(PINO_PRESENCA);
    mqtt.publish(TOPIC_PUBLISH_PRESENCA, String(presenca).c_str());
  }

  delay(20);
}
