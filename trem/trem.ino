#include <WiFi.h>              
#include <WiFiClientSecure.h>    
#include <PubSubClient.h>        


#define LED_VERDE 22
#define LED_VERMELHO 23

// Credenciais wifi
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* BROKER = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int   PORT   = 8883; 

const char* MQTT_USER = "Trem_SA";
const char* MQTT_PASS = "Trem1234";

// Tópicos de envio e recebimento
const char* TOPIC = "projeto/trem/velocidade";

// Criação do wificlient clientsecure MQTT
WiFiClientSecure clientSecure;
PubSubClient mqtt(clientSecure);



// escuta e responde assim que chega alguma informaçõ

void callback(char* topic, byte* payload, unsigned int length) {
  String mensagem = "";

  // Monta a string recebida byte a byte
  for (int i = 0; i < length; i++) {
    mensagem += (char)payload[i];
  }

  Serial.print("Recebido: ");
  Serial.println(mensagem);

  // Converte para inteiro
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

    // Cria um ID aleatório para o cliente MQTT
    String clientId = "Trem_";
    clientId += String(random(0xffff), HEX);

    // Tenta conectar usando user e senha do MQTT
    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("Conectado!");

      // Assina o tópico após conectar
      mqtt.subscribe(TOPIC);

    } else {
      Serial.print("Falhou. Código: ");
      Serial.println(mqtt.state());
      delay(1500);
    }
  }
}


// Configurações ESP32
void setup() {
  Serial.begin(115200); 

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  // Conecta ao WiFi
  Serial.print("Conectando ao WiFi");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // desabilita client secure
  clientSecure.setInsecure();

  // Configura o servidor MQTT e callback
  mqtt.setServer(BROKER, PORT);
  mqtt.setCallback(callback);

  Serial.println("Conectando ao broker MQTT...");
}



// Loop q reconecta mqtt

void loop() {

  if (!mqtt.connected()) {
    reconnect();
  }

  // Mantém o cliente MQTT ativo 
  mqtt.loop();

  // Se digitar algo no monitor serial → publica no tópico MQTT
  if (Serial.available() > 0) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();                                 
    mqtt.publish(TOPIC, msg.c_str());        
  }
}
