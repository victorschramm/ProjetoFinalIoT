/*
  Projeto Final IoT - ESP32 + MQTT
  Lê um potenciômetro (ADC) e converte o valor via regra de três
  para temperatura e umidade simuladas. Envia JSON para o tópico
  "ProjetoFinalIot" no broker público "broker.hivemq.com".

  Antes de compilar, edite `include/credentials.h` com sua rede WiFi.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"

const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "ProjetoFinalIot";

WiFiClient espClient;
PubSubClient client(espClient);

// OBS: GPIO4 é parte do ADC2 no ESP32. ADC2 é compartilhado com o WiFi
// e leituras analógicas em ADC2 podem ser imprecisas ou indisponíveis
// quando o WiFi está em uso. Se você notar leituras inconsistentes,
// prefira um pino ADC1 (por exemplo 32,33,34,35,36,39).
const int POT_PIN = 32; // ADC2_CH0 (pino do potenciômetro)
const unsigned long PUBLISH_INTERVAL = 5000; // ms

unsigned long lastPublish = 0;

// Mapeamento em ponto flutuante (regra de 3)
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup_wifi() {
  delay(10);
  Serial.print("Conectando em WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  const unsigned long timeout = 20000; // 20s
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(500);
    Serial.print('.');
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Falha ao conectar no WiFi (timeout). Continuando sem rede.");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar MQTT...");
    String clientId = "esp32-" + String((uint32_t)ESP.getEfuseMac());
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println("; nova tentativa em 2s");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  analogReadResolution(12); // 0..4095

  setup_wifi();

  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnect();
  }

  client.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    int raw = analogRead(POT_PIN); // 0 - 4095

    // Convertendo com regra de três (mapa linear)
    // Potenciômetro: 0-4095 -> 0-100%
    float potValue = mapf((float)raw, 0.0f, 4095.0f, 0.0f, 100.0f);
    
    // Temperatura: 0-4095 -> 15-35°C
    float temperature = mapf((float)raw, 0.0f, 4095.0f, 15.0f, 35.0f);
    
    // Umidade: 0-4095 -> 20-90%
    float humidity = mapf((float)raw, 0.0f, 4095.0f, 20.0f, 90.0f);

    // Monta JSON com os 3 valores
    char payload[160];
    snprintf(payload, sizeof(payload), 
      "{\"Potenciometro\":%.2f,\"Temp\":%.2f,\"Umidade\":%.2f}", 
      potValue, temperature, humidity);

    Serial.print("Publicando: ");
    Serial.println(payload);

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      client.publish(mqtt_topic, payload);
    } else {
      Serial.println("Não publicado (sem conexão MQTT/WiFi)");
    }
  }
}