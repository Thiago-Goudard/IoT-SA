#include <WiFiClientSecure.h>   // Biblioteca para conexão WiFi segura (SSL/TLS)
#include <PubSubClient.h>       // Biblioteca para MQTT
#include <WiFi.h>               // Biblioteca principal do WiFi no ESP32
#include <ESP32Servo.h>         // Biblioteca para controlar servos no ESP32

#define PINO_LED 2              // LED no pino 2
#define TRIG 26                 // Pino TRIG do ultrassom
#define ECHO 25                 // Pino ECHO do ultrassom
#define PINO_SERVO 19           // ⚠️ Esse nome se repete, erro no código
#define PINO_SERVO 18           // ⚠️ O segundo substitui o primeiro → deve corrigir
#define PINO_PRESENCA 14        // Sensor PIR no pino 14

WiFiClientSecure client;        // Cliente WiFi seguro
PubSubClient mqtt(client);      // Cliente MQTT usando WiFi seguro

Servo servo3;                   // Servo 1
Servo servo4;                   // Servo 2 

const char* SSID = "FIESC_IOT_EDU";     // Nome da rede WiFi
const char* PASS = "8120gv08";          // Senha da rede

// Dados do broker MQTT HiveMQ Cloud
const char* BROKER_URL  = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int   BROKER_PORT = 8883;         // Porta segura TLS
const char* BROKER_USER = "Placa_3";
const char* BROKER_PASS = "Placa123";

// Tópicos MQTT de envio
const char* TOPIC_PUBLISH_PRESENCA = "Projeto S3 Presenca3";
const char* TOPIC_PUBLISH_OBJETO   = "Projeto S3 Ultrassom3";

// Tópico de entrada (controle de LED)
const char* TOPICO_SUBSCRIBE = "S1 iluminacao";

// Tópicos da S2 para acionar servos
const char* TOPIC_PUBLISH_1 = "Projeto S2 Distancia1";
const char* TOPIC_PUBLISH_2 = "Projeto S2 Distancia2";

unsigned long lastPublish = 0;  // Controle do tempo de envio
int publishInterval = 3000;     // Envia a cada 3 segundos

// Função que mede distância com sensor ultrassônico
long medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);       // Limpa TRIG
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);      // Pulso de 10μs
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 30000);  // Tempo do eco
  long distancia = (duracao * 0.034) / 2;        // Converte para cm

  return distancia;
}

// Função chamada sempre que chega mensagem MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];    // Converte bytes para string
  }

  // Controle do LED via MQTT
  if (mensagem == "acender") {
    digitalWrite(PINO_LED, HIGH);
  } else if (mensagem == "apagar") {
    digitalWrite(PINO_LED, LOW);
  }

  // Controle do servomotor conforme mensagens da S2
  else if (String(topic) == TOPIC_PUBLISH_1) {
    if (mensagem == "objeto_proximo") {
      servo3.write(90);              // Servo 1 abre
    } else if (mensagem == "objeto_longe") {
      servo3.write(45);              // Servo 1 fecha
    }

  } else if (String(topic) == TOPIC_PUBLISH_2) {
    if (mensagem == "objeto_proximo") {
      servo4.write(90);              // Servo 2 abre
    } else if (mensagem == "objeto_longe") {
      servo4.write(45);              // Servo 2 fecha
    }
  }

  Serial.println(mensagem);          // Exibe mensagem recebida
}

// Conecta ao WiFi
void conectarWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nWiFi conectado!");
}

// Conecta ao servidor MQTT
void conectarMQTT() {
  mqtt.setServer(BROKER_URL, BROKER_PORT);   // Define broker
  client.setInsecure();                      // Não verifica certificado
  mqtt.setCallback(callback);                // Função de retorno

  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker...");

    String clientId = "S3_" + String(random(0xffff), HEX);  // ID único

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");

      // Inscreve nos tópicos
      mqtt.subscribe(TOPICO_SUBSCRIBE);
      mqtt.subscribe(TOPIC_PUBLISH_1);
      mqtt.subscribe(TOPIC_PUBLISH_2);
      mqtt.subscribe("Projeto/S3/Controle");

    } else {
      Serial.print("Falha. Código: ");
      Serial.println(mqtt.state());
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

  servo3.attach(PINO_SERVO);      
  servo3.write(0);

  conectarWiFi();
  conectarMQTT();
}

void loop() {
  if (!mqtt.connected()) conectarMQTT();  // Reconnect automático
  mqtt.loop();                            // Processa MQTT

  long distancia = medirDistancia(TRIG, ECHO);
  Serial.println(distancia);

  // Publica mensagem dependendo da distância
  if (distancia > 0 && distancia < 10) {
    mqtt.publish(TOPIC_PUBLISH_1, "objeto_proximo");
  } else if (distancia > 10) {
    mqtt.publish(TOPIC_PUBLISH_2, "objeto_longe");
  }

  // Publica presença a cada 3 segundos
  unsigned long agora = millis();
  if (agora - lastPublish >= publishInterval) {
    lastPublish = agora;
    int presenca = digitalRead(PINO_PRESENCA);
    mqtt.publish(TOPIC_PUBLISH_PRESENCA, String(presenca).c_str());
    Serial.print("Presença publicada: ");
    Serial.println(presenca);
  }

  delay(20);   // Pequeno delay para estabilizar
}
