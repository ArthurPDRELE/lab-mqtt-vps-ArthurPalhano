#include <WiFi.h>
#include <PubSubClient.h>

// --- Configurações de Rede e Broker ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
const char* mqtt_server = "broker.emqx.io"; 
const int mqtt_port = 1883; 

// --- Pinos e Variáveis de Controle ---
const int pinoSensor = 23; 
int estadoAnterior = -1;
unsigned long tempoAbertura = 0;
unsigned long ultimoCheckConexao = 0;
bool alertaTempoExcedidoEnviado = false;

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
      Serial.print("Tentando conexão MQTT...");
      if (client.connect("ESP32_Estoque_Arthur_123")) {
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
  pinMode(pinoSensor, INPUT_PULLUP);
}

void loop() {
  reconnect(); 
  client.loop();

  int estadoAtual = digitalRead(pinoSensor); 

  // --- Lógica 1: Mudança de Estado ---
  if (estadoAtual != estadoAnterior) {
    client.publish("empresa/estoque/porta/status", estadoAtual == HIGH ? "1" : "0");
    
    if (estadoAtual == HIGH) {
      Serial.println("Porta ABERTA - Cronômetro iniciado.");
      tempoAbertura = millis(); 
      alertaTempoExcedidoEnviado = false;
    } else {
      Serial.println("Porta FECHADA.");
      tempoAbertura = 0; 
    }
    estadoAnterior = estadoAtual;
  }

  // --- Lógica 2: Alerta de Tempo Excedido (60s) ---
  if (estadoAtual == HIGH && tempoAbertura > 0 && !alertaTempoExcedidoEnviado) {
    if (millis() - tempoAbertura >= 60000) { 
      client.publish("empresa/estoque/sistema/alerta", "ALERTA: PORTA ABERTA HA MAIS DE 1 MINUTO");
      Serial.println("!!! ALERTA ENVIADO !!!");
      alertaTempoExcedidoEnviado = true; 
    }
  }
}
