#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define LED_VERDE 22
#define LED_VERMELHO 23

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* BROKER = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int   PORT   = 8883;

const char* MQTT_USER = "Trem_SA";
const char* MQTT_PASS = "Tream1234";

const char* TOPIC = "projeto/trem/velocidade";

WiFiClientSecure clientSecure;
PubSubClient mqtt(clientSecure);

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";

  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Recebido: ");
  Serial.println(mensagem);

  int val = mensagem.toInt();

  if (val > 0) {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
  } else if (val < 0) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
  } else {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, LOW);
  }
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Tentando conectar... ");

    String clientId = "Trem_";
    clientId += String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado!");
      mqtt.subscribe(TOPIC);
    } else {
      Serial.print("Falhou. Código: ");
      Serial.println(mqtt.state());
      delay(1500);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  Serial.print("Conectando ao WiFi");

  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");

  clientSecure.setInsecure();

  mqtt.setServer(BROKER, PORT);
  mqtt.setCallback(callback);

  Serial.println("Conectando ao broker MQTT...");
}




void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop();
  
/* Esse loop mantém o MQTT funcionando.
 Se desconectar: chama reconnect()
mqtt.loop() mantém o cliente vivo, lendo mensagens*/

  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    mqtt.publish(TOPIC, msg.c_str());
  }
}

/*
  Se você digitar algo no monitor serial:
 Ele lê a mensagem
Remove espaços extras
Publica no tópico MQTT 
*/
