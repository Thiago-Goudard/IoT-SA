#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;
PubSubClient mqtt(client);

const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* URL = "test.mosquitto.org";
const int PORT = 1883;

const char* MyTopic = "Gabriel";
const char* OtherTopic = "Thiago";

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi conectado!");
  mqtt.setServer(URL, PORT);
  while (!mqtt.connected()) {
    String clientId = "Gabriel_";
    clientId += String(random(0xffff), HEX);
    mqtt.connect(clientId.c_str());
    delay(200);
  }
  Serial.println("Conectado ao broker!");
}

void loop() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();

    if (comando == "Ligue" || comando == "Apague") {
      mqtt.publish(OtherTopic, comando.c_str());
      Serial.println("Comando enviado: " + comando);
    } else {
      Serial.println("Digite 'Ligue' ou 'Apague'");
    }
  }
  mqtt.loop();
}
