#include <WiFi.h>
#include <PubSubClient.h>

// --- Configurações de Rede e Broker (Conforme seu Escopo e Guia) ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
const char* mqtt_server = "134.199.142.253"; // Seu IP da VPS DigitalOcean
const int mqtt_port = 1883; 

// --- Pinos e Variáveis de Controle ---
const int pinoSensor = 23; 
int estadoAnterior = -1;
unsigned long tempoAbertura = 0;
unsigned long ultimoCheckConexao = 0;
bool alertaTempoExcedidoEnviado = false;

WiFiClient espClient;
PubSubClient client(espClient);

// Função para conectar ao Wi-Fi do Wokwi
void setup_wifi() {
  Serial.println("\nConectando Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
}

// Função de reconexão MQTT não bloqueante (Uso de millis)
void reconnect() {
  if (!client.connected()) {
    unsigned long agora = millis();
    // Tenta reconectar a cada 5 segundos para não travar o processador
    if (agora - ultimoCheckConexao > 5000) { 
      ultimoCheckConexao = agora;
      Serial.print("Tentando conexão MQTT na VPS...");
      
      // ID único para evitar conflitos no Broker
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
  
  // Configuração do Servidor e Porta (1883 conforme guia de lab)
  client.setServer(mqtt_server, mqtt_port);
  
  // Pino 23 configurado com resistor de pull-up interno
  pinMode(pinoSensor, INPUT_PULLUP);
}

void loop() {
  reconnect(); 
  client.loop(); // Mantém o "keep-alive" com o Broker

  // Leitura do estado (Wokwi Switch para GND: HIGH = Aberto | LOW = Fechado)
  int estadoAtual = digitalRead(pinoSensor); 

  // --- Lógica 1: Mudança de Estado (Detecção de Abertura/Fechamento) ---
  if (estadoAtual != estadoAnterior) {
    // Publica status conforme os tópicos do seu escopo
    client.publish("empresa/estoque/porta/status", estadoAtual == HIGH ? "1" : "0");
    
    if (estadoAtual == HIGH) {
      Serial.println("Porta ABERTA - Cronômetro de segurança iniciado.");
      tempoAbertura = millis(); // Salva o tempo de início usando millis()
      alertaTempoExcedidoEnviado = false;
    } else {
      Serial.println("Porta FECHADA.");
      tempoAbertura = 0; 
    }
    estadoAnterior = estadoAtual;
  }

  // --- Lógica 2: Alerta de Tempo Excedido (60 segundos) ---
  // Verifica o tempo sem usar delay(), permitindo que o ESP32 continue operando
  if (estadoAtual == HIGH && tempoAbertura > 0 && !alertaTempoExcedidoEnviado) {
    if (millis() - tempoAbertura >= 60000) { 
      client.publish("empresa/estoque/sistema/alerta", "ALERTA: PORTA ABERTA HA MAIS DE 1 MINUTO");
      Serial.println("!!! ALERTA DE SEGURANÇA ENVIADO PARA A VPS !!!");
      alertaTempoExcedidoEnviado = true; 
    }
  }
}
