#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String URL   = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int PORT     = 8883;
const String broker_USR   = "Placa_1";
const String broker_PASS  = "Placa123";

const String MyTopic = "sensor/comando";
const String OtherTopic = "sensor/distancia";

const int trigPin = 22;
const int echoPin = 23;
const int ledPin  = 19;

long duracao;
float distancia;
bool detectou = false;

void setup() {
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Conectando ao WiFi");
  WiFi.begin(SSID.c_str(), PASS.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }

  Serial.println("\nConectado com sucesso!");
  client.setInsecure();
  Serial.println("Conectando ao Broker");

  mqtt.setServer(URL.c_str(), PORT);
  mqtt.setCallback(callback);

  while (!mqtt.connected()) {
    String ID = "s1_";
    ID += String(random(0xffff), HEX);
    mqtt.connect(ID.c_str(), broker_USR.c_str(), broker_PASS.c_str());
    Serial.print(".");
    delay(200);
  }

  mqtt.subscribe(MyTopic.c_str());
  Serial.println("\nConectado com sucesso ao broker!");
  digitalWrite(ledPin, LOW);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duracao = pulseIn(echoPin, HIGH);
  distancia = duracao * 0.0343 / 2;

  if (distancia > 0 && distancia < 10 && !detectou) {
    detectou = true;
    digitalWrite(ledPin, HIGH);
    mqtt.publish(OtherTopic.c_str(), "Objeto detectado!");
    Serial.println("Objeto detectado!");
  } 
  else if (distancia >= 10 && detectou) {
    detectou = false;
    digitalWrite(ledPin, LOW);
    mqtt.publish(OtherTopic.c_str(), "Área livre");
    Serial.println("Área livre");
  }

  mqtt.loop();
  delay(300);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.println("Recebido:");
  Serial.println(mensagem);

  if (mensagem.equalsIgnoreCase("acender")) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ACESO!");
  } 
  else if (mensagem.equalsIgnoreCase("apagar")) {
    digitalWrite(ledPin, LOW);
    Serial.println("LED APAGADO!");
  } 
  else {
    Serial.println("Mensagem não reconhecida.");
  }
}