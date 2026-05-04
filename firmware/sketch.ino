#include <WiFi.h>
#include <PubSubClient.h>

// --- Configurações de Rede e Broker ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
const char* mqtt_server = "134.199.142.253"; 
const int mqtt_port = 1883; 

// --- Definição dos Pinos ---
const int pinoPorta = 23;      // Reed Switch
const int pinoPIR = 13;        // Sensor de Movimento
const int pinoLDR = 34;        // Sensor de Luminosidade (ADC)
const int pinoRele = 2;        // Atuador da Luz (LED no ESP32 ou Relé)

// --- Variáveis de Controle de Tempo e Estado ---
int estadoPortaAnterior = -1;
bool movimentoAnterior = false;
bool luzAcesa = true;

unsigned long tempoAberturaPorta = 0;
unsigned long ultimoMovimento = 0;
unsigned long ultimoCheckConexao = 0;
unsigned long ultimoEnvioLDR = 0;

bool alertaPortaExcedidoEnviado = false;
const long tempoLimitePorta = 60000;    // 1 minuto
const long tempoLimiteLuz = 600000;    // 10 minutos

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.println("\nConectando Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
}

void reconnect() {
  if (!client.connected()) {
    unsigned long agora = millis();
    if (agora - ultimoCheckConexao > 5000) { 
      ultimoCheckConexao = agora;
      Serial.print("Tentando conexão MQTT na VPS...");
      if (client.connect("ESP32_Estoque_Arthur_Palhano")) {
        Serial.println("Conectado!");
      } else {
        Serial.print("Falhou, rc=");
        Serial.println(client.state());
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  
  pinMode(pinoPorta, INPUT_PULLUP);
  pinMode(pinoPIR, INPUT);
  pinMode(pinoRele, OUTPUT);
  
  digitalWrite(pinoRele, HIGH); // Inicia com a luz acesa
  ultimoMovimento = millis();   // Inicia o cronômetro de vacância
}

void loop() {
  reconnect(); 
  client.loop();

  unsigned long tempoAtual = millis();

  // --- 1. Lógica do Sensor de Porta ---
  int estadoPorta = digitalRead(pinoPorta); 
  if (estadoPorta != estadoPortaAnterior) {
    client.publish("empresa/estoque/porta/status", estadoPorta == HIGH ? "aberto" : "fechado");
    if (estadoPorta == HIGH) {
      tempoAberturaPorta = tempoAtual;
      alertaPortaExcedidoEnviado = false;
    }
    estadoPortaAnterior = estadoPorta;
  }

  if (estadoPorta == HIGH && !alertaPortaExcedidoEnviado) {
    if (tempoAtual - tempoAberturaPorta >= tempoLimitePorta) {
      client.publish("empresa/estoque/sistema/alerta", "ALERTA CRITICO: PORTA ABERTA > 60s");
      alertaPortaExcedidoEnviado = true;
    }
  }

  // --- 2. Lógica de Movimento (PIR) e Automação de Luz ---
  bool movimentoAtual = digitalRead(pinoPIR);

  if (movimentoAtual) {
    ultimoMovimento = tempoAtual; // Reseta o cronômetro de 10 min
    if (!movimentoAnterior) {
      client.publish("empresa/estoque/presenca/alerta", "ALERTA: Movimento detectado!");
      movimentoAnterior = true;
    }
    
    // Se a luz estava apagada, religa ao detectar movimento
    if (!luzAcesa) {
      digitalWrite(pinoRele, HIGH);
      luzAcesa = true;
      client.publish("empresa/estoque/luz/automacao", "Luz ligada por detecção de presença");
    }
  } else {
    if (movimentoAnterior) {
      client.publish("empresa/estoque/presenca/alerta", "Ambiente normal: sem movimento.");
      movimentoAnterior = false;
    }
  }

  // Desligamento automático (10 minutos sem movimento)
  if (luzAcesa && (tempoAtual - ultimoMovimento >= tempoLimiteLuz)) {
    digitalWrite(pinoRele, LOW);
    luzAcesa = false;
    client.publish("empresa/estoque/luz/automacao", "Luzes desligadas por inatividade (10 min).");
  }

  // --- 3. Envio Periódico de Luminosidade (LDR) ---
  if (tempoAtual - ultimoEnvioLDR > 30000) { // Envia a cada 30 segundos
    int valorLDR = analogRead(pinoLDR);
    char buffer[10];
    sprintf(buffer, "%d", valorLDR);
    client.publish("empresa/estoque/luz/nivel", buffer);
    ultimoEnvioLDR = tempoAtual;
  }
}
