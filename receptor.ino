#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;
PubSubClient mqtt(client);

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* URL = "test.mosquitto.org";
const int PORT = 1883;

const char* MyTopic = "Thiago";
const int LED = 2; // use GPIO2 em vez de 8

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem;
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }
  mensagem.trim();
  Serial.println("Mensagem recebida: " + mensagem);

  if (mensagem.equalsIgnoreCase("Ligue")) {
    digitalWrite(LED, HIGH);
    Serial.println("LED ligado!");
  } else if (mensagem.equalsIgnoreCase("Apague")) {
    digitalWrite(LED, LOW);
    Serial.println("LED desligado!");
  }
}

void reconnect() {
  while (!mqtt.connected()) {
    String clientId = "Thiago_";
    clientId += String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str())) {
      mqtt.subscribe(MyTopic);
      Serial.println("Reconectado ao broker!");
    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  WiFi.begin(SSID, PASS);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi conectado!");
  mqtt.setServer(URL, PORT);
  mqtt.setCallback(callback);
}

void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();
}