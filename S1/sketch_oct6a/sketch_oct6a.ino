#include <WiFi.h>

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

void setup() {
  Serial.begin(115200);
  Serial.prntln("Conectando ao Wifi");
  WiFi.begin(SSID,PASS);
  while (wiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectando co sucesso!");
}


void loop() {
  // put your main code here, to run repeatedly:

}
