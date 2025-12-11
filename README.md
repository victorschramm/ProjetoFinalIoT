# Projeto Final IoT - ESP32 + MQTT + DHT11

## Descrição

Este projeto utiliza um **ESP32** com sensor **DHT11** para leitura de temperatura e umidade, enviando os dados via **MQTT** para o broker público `broker.hivemq.com`.

### Funcionalidades
- Leitura de temperatura (°C) e umidade (UR %) do sensor DHT11
- Envio de dados via MQTT no formato JSON
- Sincronização de horário via NTP para timestamp real
- Reconexão automática WiFi e MQTT

### Formato JSON
Cada leitura é enviada como uma mensagem separada:

```json
{"id_sensor":1,"valor":25.50,"tipo_leitura":"temperatura","unidade":"Celsius","timestamp":"2025-12-10 14:30:45"}
{"id_sensor":1,"valor":60.00,"tipo_leitura":"umidade","unidade":"UR %","timestamp":"2025-12-10 14:30:45"}
```

## Hardware

### Componentes
- ESP32 DevKit
- Sensor DHT11
- Resistor 10kΩ (pull-up, opcional mas recomendado)

### Conexões DHT11
| DHT11 | ESP32 |
|-------|-------|
| VCC   | 3.3V  |
| DATA  | GPIO 32 |
| GND   | GND   |

> **Nota:** Recomenda-se usar um resistor de 10kΩ entre VCC e DATA.

## Configuração

1. Abra `include/credentials.h` e preencha suas credenciais WiFi:

```cpp
#define WIFI_SSID "SuaRede"
#define WIFI_PASS "SuaSenha"
```

2. O tópico MQTT padrão é `ProjetoFinalIot` no broker `broker.hivemq.com`

## Compilar e Enviar (PlatformIO)

1. Abra o projeto no VS Code com a extensão PlatformIO
2. Clique em **Upload** (ícone da seta) ou execute:

```powershell
pio run --target upload
```

## Monitor Serial

```powershell
pio device monitor --baud 115200
```

## Testar MQTT

Usando `mosquitto_sub`:

```powershell
mosquitto_sub -h broker.hivemq.com -t ProjetoFinalIot -v
```

Ou acesse o [HiveMQ WebSocket Client](http://www.hivemq.com/demos/websocket-client/) e inscreva-se no tópico `ProjetoFinalIot`.

## Observações

- O broker `broker.hivemq.com` é público e não usa autenticação; para produção use broker seguro com TLS e autenticação
- Intervalo de publicação: 5 segundos
- Fuso horário configurado: GMT-3 (Brasil)
- O DHT11 tem precisão de ±2°C para temperatura e ±5% para umidade
