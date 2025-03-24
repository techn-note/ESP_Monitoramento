# ESP32 e MQTT para Monitoramento

Este projeto utiliza um ESP32 para monitorar sensores e controlar barras de LED com base nas leituras, enviando os dados via MQTT.

## 📌 Funcionalidades
- Leitura de sensores (temperatura, distância e TDS)
- Controle de barras de LED baseado nas leituras
- Envio de dados para um broker MQTT
- Reconexão automática do Wi-Fi e MQTT em caso de falha

## 🛠️ Hardware Utilizado
- ESP32 (Espressif ou similar)
- Barras de LED
- Sensor de temperatura
- Sensor de distância (HC-SR04)
- Sensor de TDS
- Fonte de alimentação adequada

## 📡 Configuração do MQTT
1. Configure o **broker MQTT** no código.
2. Defina os **tópicos MQTT** conforme necessário.
3. O envio de dados ocorre a cada 10 segundos.

## ⚡ Solução para Oscilações nos LEDs
Caso os LEDs oscilem quando a conexão MQTT está ativa:
- Aumente o intervalo de atualização dos LEDs.
- Priorize a atualização dos LEDs antes do processamento MQTT.
- Reduza a frequência de leitura dos sensores.

## 🚀 Como Usar
1. Compile e carregue o código no ESP32.
2. Conecte os sensores e LEDs conforme o esquema.
3. Inicie um broker MQTT e visualize os dados.
4. Ajuste parâmetros conforme necessário.

## 📝 Licença
Este projeto é de código aberto e pode ser modificado conforme necessário.

