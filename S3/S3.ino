#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ========================
//  WIFI – S3
// ========================
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

// ========================
//  MQTT – HiveMQ Cloud
// ========================
const char* MQTT_SERVER = "810a9479164b4b4d81ba1c4879369294.s1.eu.hivemq.cloud";
const int   MQTT_PORT   = 8883;
const char* MQTT_USER   = "Placa_3";
const char* MQTT_PASS   = "Placa123";

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

// ========================
//  PINOS DO S3
// ========================
#define TRIG_PIN 12
#define ECHO_PIN 13

#define LED_PIN 25
#define SERVO1_PIN 32
#define SERVO2_PIN 33

Servo servo1;
Servo servo2;

bool estadoPresenca = false;

// =========================================================
//  CALLBACK – RECEBE COMANDOS MQTT
// =========================================================
void callback(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) msg += (char)payload[i];

    if (String(topic) == "S1/Iluminacao") {
        if (msg == "Escuro") digitalWrite(LED_PIN, HIGH);
        else digitalWrite(LED_PIN, LOW);
    }

    if (String(topic) == "S3/Servo1") servo1.write(msg.toInt());
    if (String(topic) == "S3/Servo2") servo2.write(msg.toInt());
}

// =========================================================
//  CONECTAR AO MQTT
// =========================================================
void conectarMQTT() {
    while (!mqtt.connected()) {
        String id = "S3-" + String(random(9999));
        mqtt.connect(id.c_str(), MQTT_USER, MQTT_PASS);
        delay(300);
    }

    mqtt.subscribe("S1/Iluminacao");
    mqtt.subscribe("S3/Servo1");
    mqtt.subscribe("S3/Servo2");
}

// =========================================================
//  SENSOR ULTRASSÔNICO
// =========================================================
float distanciaCM() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
    return duracao * 0.0343 / 2;
}

// =========================================================
//  SETUP
// =========================================================
void setup() {
    Serial.begin(115200);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    servo1.setPeriodHertz(50);
    servo2.setPeriodHertz(50);
    servo1.attach(SERVO1_PIN, 500, 2400);
    servo2.attach(SERVO2_PIN, 500, 2400);

    WiFi.begin(SSID, PASS);
    while (WiFi.status() != WL_CONNECTED) delay(300);

    secureClient.setInsecure();
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(callback);

    conectarMQTT();
}

// =========================================================
//  LOOP PRINCIPAL
// =========================================================
void loop() {
    if (!mqtt.connected()) conectarMQTT();
    mqtt.loop();

    float dist = distanciaCM();

    // ---------- PRESENÇA DETECTADA ----------
    if (dist > 0 && dist < 10 && !estadoPresenca) {
        estadoPresenca = true;

        mqtt.publish("S3/Presenca", "1");
        mqtt.publish("S3/Status", "Presenca detectada - Servos acionados");

        Serial.println(">>> PRESENÇA DETECTADA!");

        servo1.write(180);
        servo2.write(0);
    }

    // ---------- ÁREA LIVRE ----------
    if ((dist >= 10 || dist == 0) && estadoPresenca) {
        estadoPresenca = false;

        mqtt.publish("S3/Presenca", "0");
        mqtt.publish("S3/Status", "Area livre - Servos retornaram");

        Serial.println(">>> ÁREA LIVRE!");

        servo1.write(0);
        servo2.write(180);
    }

    delay(200);
}

