#include <WiFi.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include "leds.h"

const char* mqtt_server = "seu broker";
const int mqtt_port = porta;
const char* mqtt_topic = "coleção_1";
const char* mqtt_log_topic = "coleção_2";
const char* mqtt_user = "seu usuario";
const char* mqtt_password = "sua senha";

WiFiClient espClient;
PubSubClient client(espClient);

#define SD_CS_PIN 5
#define TEMP_PIN 34
#define TRIG_PIN 13
#define ECHO_PIN 12
#define TDS_PIN 32
#define LED_PIN 2

//  sensor NTC10K
double Vs = 3.3;
double R1 = 10000;
double Beta = 3950;
double To = 298.15;
double Ro = 10000;
double adcMax = 4095.0;

 
const float TDS_FACTOR = 0.5;

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long ultimaAtualizacaoLCD = 0;
unsigned long ultimaAtualizacaoLEDs = 0;
unsigned long ultimaAtualizacaoMQTT = 0;
const long intervaloLCD = 3000;
const long intervaloLEDs = 500;
const long intervaloMQTT = 10000;

bool conectadoWiFi = false, conectadoMQTT = false;
unsigned long tempoDesconexao = 0;

void salvarNoSD(const String &dados) {
    File file = SD.open("/log.txt", FILE_WRITE); 
    if (file) {
        file.println(dados);
        file.close();
        Serial.println("Salvo no SD: " + dados);
    } else {
        Serial.println("Falha ao escrever no SD");
    }
}

void reconnectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi não está conectado, aguardando reconexão...");
        return;
    }

    int tentativas = 0;
    while (!client.connected() && tentativas < 2) {
        Serial.print("Conectando ao MQTT...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("Conectado ao MQTT!");
            digitalWrite(LED_PIN, LOW);
            conectadoMQTT = true;
            tempoDesconexao = 0;  
            return;
        } else {
            Serial.print("Falha, rc=");
            Serial.print(client.state());
            Serial.println(" Tentando novamente...");
            tentativas++;
            digitalWrite(LED_PIN, HIGH);
            delay(3000);  /
        }
    }

    Serial.println("Falha ao conectar ao MQTT. Tentando mais tarde...");
    conectadoMQTT = false;
    if (tempoDesconexao == 0) tempoDesconexao = millis();
}

float lerTemperatura() {
  int leitura = analogRead(TEMP_PIN);
  double Vout = leitura * Vs / adcMax;
  double Rt = R1 * Vout / (Vs - Vout);
  double T = 1 / (1 / To + log(Rt / Ro) / Beta);
  return T - 273.15;
}

float lerDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH);
  return duracao * 0.034 / 2;
}

int lerTDS() {
  int leitura = analogRead(TDS_PIN);
  float tensao = leitura * (3.3 / 4095.0);
  return (int)((tensao * TDS_FACTOR) * 1000);
}

String montarJSON(float temperatura, float distancia, int tds) {
  char buffer[100];
  sprintf(buffer, "{\"temperatura\":%.2f,\"distancia\":%.2f,\"tds\":%d}", temperatura, distancia, tds);
  return String(buffer);
}

void enviarMQTT(float temperatura, float distancia, int tds) {
  String dadosJSON = montarJSON(temperatura, distancia, tds);
  if (client.connected()) {
    client.publish(mqtt_topic, dadosJSON.c_str());
    Serial.println("Dados enviados: " + dadosJSON);
  } else {
    Serial.println("MQTT desconectado! Salvando no SD...");
    salvarNoSD(dadosJSON);

    if (tempoDesconexao == 0) {
            tempoDesconexao = millis();
        }
  }
}

void mostrarBoasVindas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bem-vindo ao");
  lcd.setCursor(0, 1);
  lcd.print("AquaSense!");
  delay(3000);
  lcd.clear();
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SD_CS_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Erro ao inicializar o cartão SD. Tentando novamente...");
        delay(2000);
        if (!SD.begin(SD_CS_PIN)) {
            Serial.println("Falha crítica no SD. Reiniciando...");
            ESP.restart();
        }
    }

  WiFiManager wifiManager;
  wifiManager.autoConnect("AquaSense_AP");
  Serial.println("WiFi conectado.");
  client.setServer(mqtt_server, mqtt_port);

    enviarLogMQTT();
    
  inicializarLEDs();
  Serial.println("LEDs inicializados.");

  mostrarBoasVindas();
}

void enviarLogMQTT() {
    if (!SD.exists("/log.txt")) {
        Serial.println("Arquivo de log não encontrado.");
        return;
    }

    File file = SD.open("/log.txt", FILE_READ);
    if (!file) {
        Serial.println("Erro ao abrir o arquivo de log.");
        return;
    }

    while (file.available()) {
        String linha = file.readStringUntil('\n');
        if (client.connected()) {
            client.publish(mqtt_log_topic, linha.c_str());
            Serial.println("Log enviado: " + linha);
            delay(500); 
        } else {
            Serial.println("MQTT desconectado. Não foi possível enviar o log.");
            digitalWrite(LED_PIN, HIGH);
            break;
        }
    }

    file.close();
}

void loop() {
    unsigned long agora = millis();

    float temperatura = lerTemperatura();
    float distancia = lerDistancia();
    int tds = lerTDS();

    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado! Tentando reconectar...");
    conectadoWiFi = false;
    digitalWrite(LED_PIN, HIGH);
    if (tempoDesconexao == 0) tempoDesconexao = millis();
    
    WiFi.reconnect();  
    delay(100);
} else {
    if (!conectadoWiFi) {
        Serial.println("WiFi reconectado!");
        digitalWrite(LED_PIN, LOW);
        conectadoWiFi = true;
        tempoDesconexao = 0;
    }
}

    client.loop();

    
if (agora - ultimaAtualizacaoLEDs >= intervaloLEDs) {
    ultimaAtualizacaoLEDs = agora;
    atualizarLEDs(barra1, map(temperatura, 0, 40, 0, 100));
    atualizarLEDs(barra2, map(distancia, 0, 100, 0, 100)); 
    atualizarLEDs(barra3, map(tds, 0, 100, 0, 100));
}


if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
} else if (!client.connected()) {
    reconnectMQTT();
}


    static int estadoLCD = 0;
    static unsigned long tempoLCD = 0;

    if (agora - tempoLCD >= intervaloLCD) {
        tempoLCD = agora;
        lcd.clear();

        switch (estadoLCD) {
            case 0:
                lcd.setCursor(0, 0);
                lcd.print("Temp: ");
                lcd.print(temperatura, 1);
                lcd.print(" C");
                break;

            case 1:
                lcd.setCursor(0, 0);
                lcd.print("Nivl: ");
                lcd.print(distancia, 1);
                lcd.print(" cm");
                break;

            case 2:
                lcd.setCursor(0, 0);
                lcd.print("Solidos: ");
                lcd.print(tds, 1);
                lcd.print(" ppm");
                break;

            case 3:
                if (distancia > 100) {
                    lcd.setCursor(0, 0);
                    lcd.print("Risco: Nivel baixo");
                } else if (temperatura > 32) {
                    lcd.setCursor(0, 0);
                    lcd.print("Risco: Temp. alta");
                } else if (tds > 100) {
                    lcd.setCursor(0, 0);
                    lcd.print("Risco: Solidos altos");
                }
                break;
        }

        estadoLCD = (estadoLCD + 1) % 4; 
    }

    if (agora - ultimaAtualizacaoMQTT >= intervaloMQTT) {
        ultimaAtualizacaoMQTT = agora;
        enviarMQTT(temperatura, distancia, tds);
    }
}
