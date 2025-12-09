# ProjetoFinalDSIot - ESP32 MQTT (Potenciômetro → Temperatura/Umidade)

Descrição rápida:
- Leitura de um potenciômetro conectado ao pino ADC (`GPIO4`) do ESP32.
- Converte o valor lido (0-4095) para temperatura (15°C–35°C) e umidade (20%–90%) usando regra de três.
- Envia um JSON para o tópico MQTT `ProjetoFinalIot` no broker público `broker.hivemq.com`.

Configuração:
- Abra `include/credentials.h` e preencha `WIFI_SSID` e `WIFI_PASS` com sua rede.
-- Ligue o potenciômetro: um terminal em `3.3V`, outro em `GND`, e o pino do meio (wiper) em `GPIO4`.

Aviso importante:
- `GPIO4` pertence ao ADC2 do ESP32. ADC2 é compartilhado com o WiFi e pode
	apresentar leituras imprecisas ou indisponíveis quando o WiFi estiver ativo.
	Se você observar comportamento instável, use um pino ADC1 (por exemplo
	`GPIO32`, `GPIO33`, `GPIO34`, `GPIO35`, `GPIO36`, `GPIO39`) para leituras
	analógicas confiáveis com WiFi em uso.

Compilar e enviar (PlatformIO):
1. Abra o projeto no VS Code com PlatformIO.
2. Defina as credenciais em `include/credentials.h`.
3. Clique em "Upload" (ícone da flecha) do PlatformIO ou rode no terminal do projeto:

```powershell
pio run --target upload
```

Monitor serial:

```powershell
pio device monitor --baud 115200
```

Testando o MQTT (exemplo com `mosquitto_sub`):

```powershell
mosquitto_sub -h broker.hivemq.com -t ProjetoFinalIot -v
```

Observações:
- O broker `broker.hivemq.com` é público e não usa autenticação; para produção use broker seguro com TLS e autenticação.
- Ajuste os intervalos de temperatura/umidade no `src/main.cpp` se desejar outro mapeamento.
