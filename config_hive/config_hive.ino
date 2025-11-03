#include <WiFiClientSecure.h>
 #include<PubSubClient.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

 const String SSID = "FIESC_IOT_EDU";
 const String PASS = "8120gv08";

const String URL = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int PORT = 8883;
const String USR = "Placas 1";
const String Broker_PASS= "Placa123";
const String MyTopic = "";
const String OtherTopic = "";




 void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFI");
  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso!");
  client.setInsecure();
  Serial.println("Conectando ao Broker");
  mqtt.setServer(URL.c_str(),PORT);
  while(!mqtt.connected()){
    String ID = "S2_";
    ID += String(random(0xffff),HEX);
    mqtt.connect(ID.c_str(), USR.c_str(), Broker_PASS.c_str());
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe(MyTopic.c_str());
  mqtt.setCallback(callback);
  Serial.println("\nCOnectado com sucesso ao broker!");

}


void loop() {
  String mensagem = "Thiago: ";
  if (Serial.available()>0){
    mensagem += Serial.readStringUntil('\n');
    mqtt.publish(OtherTopic.c_str(), mensagem.c_str());
  }
  mqtt.loop();
  delay(1000);
}

void callback (char* topic, byte* payload, unsigned int lenght){
  String mensagem = "";
  for(int i = 0 ;i< lenght; i++){
    mensagem += (char)payload[i];

  }
  Serial.print("Recebido:");
  Serial.println(mensagem);
}