/*
  Projeto Final IoT - ESP32 + MQTT + DHT11
  Lê temperatura e umidade do sensor DHT11 e envia via MQTT
  para o tópico "ProjetoFinalIot" no broker público "broker.hivemq.com".

  Formato JSON:
  { "id_sensor": 1, "valor": 25.5, "tipo_leitura": "temperatura", "timestamp": "..." }

  Antes de compilar, edite `include/credentials.h` com sua rede WiFi.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>
#include "credentials.h"

const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "ProjetoFinalIot";

// Configuração NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;  // Fuso horário Brasil (GMT-3)
const int daylightOffset_sec = 0;      // Sem horário de verão

WiFiClient espClient;
PubSubClient client(espClient);

// Configuração do DHT11
#define DHT_PIN 32       // GPIO 32
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

const int SENSOR_ID = 1;
const unsigned long PUBLISH_INTERVAL = 5000; // ms

unsigned long lastPublish = 0;

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

  dht.begin();
  Serial.println("DHT11 inicializado no GPIO 32");

  setup_wifi();

  // Configura NTP para obter Data/Hora
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sincronizando horário NTP...");
  
  // Aguarda sincronização
  struct tm timeinfo;
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println();
  if (retry < 10) {
    Serial.println("Horário sincronizado!");
  } else {
    Serial.println("Falha ao sincronizar horário NTP");
  }

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

    // Leitura do DHT22
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Verifica se a leitura é válida
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Erro ao ler DHT11!");
      return;
    }

    // Obtém timestamp no formato Data/Hora
    struct tm timeinfo;
    char timestamp[25];
    if (getLocalTime(&timeinfo)) {
      strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    } else {
      snprintf(timestamp, sizeof(timestamp), "N/A");
    }

    // Payload para temperatura (Celsius)
    char payloadTemp[200];
    snprintf(payloadTemp, sizeof(payloadTemp), 
      "{\"id_sensor\":%d,\"valor\":%.2f,\"tipo_leitura\":\"temperatura\",\"unidade\":\"Celsius\",\"timestamp\":\"%s\"}", 
      SENSOR_ID, temperature, timestamp);

    // Payload para umidade (UR %)
    char payloadHum[200];
    snprintf(payloadHum, sizeof(payloadHum), 
      "{\"id_sensor\":%d,\"valor\":%.2f,\"tipo_leitura\":\"umidade\",\"unidade\":\"UR %%\",\"timestamp\":\"%s\"}", 
      SENSOR_ID, humidity, timestamp);

    Serial.print("Temperatura: ");
    Serial.println(payloadTemp);
    Serial.print("Umidade: ");
    Serial.println(payloadHum);

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      client.publish(mqtt_topic, payloadTemp);
      delay(100); // Pequeno delay entre publicações
      client.publish(mqtt_topic, payloadHum);
    } else {
      Serial.println("Não publicado (sem conexão MQTT/WiFi)");
    }
  }
}