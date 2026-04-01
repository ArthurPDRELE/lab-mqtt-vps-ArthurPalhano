#include <WiFi.h>
#include <PubSubClient.h>

// --- Configurações de Rede e Broker ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 
const char* mqtt_server = "SEU_IP_DA_VPS"; 
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
  // Nota: Aqui o while ainda é aceitável no setup, pois sem Wi-Fi o app não inicia
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
}

void reconnect() {
  // Verifica a conexão sem travar o loop principal
  if (!client.connected()) {
    unsigned long agora = millis();
    if (agora - ultimoCheckConexao > 5000) { // Tenta reconectar a cada 5 segundos
      ultimoCheckConexao = agora;
      Serial.print("Tentando conexão MQTT...");
      if (client.connect("ESP32_Estoque_Arthur")) {
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
  
  // INPUT_PULLUP evita flutuação no pino se o sensor estiver desconectado
  pinMode(pinoSensor, INPUT_PULLUP);
}

void loop() {
  reconnect(); // Tenta reconectar de forma não bloqueante
  client.loop();

  // Leitura do estado (Invertida: HIGH quando o ímã se afasta/porta abre)
  int estadoAtual = digitalRead(pinoSensor); 

  // --- Lógica 1: Detecção de Mudança de Estado (Evento) ---
  if (estadoAtual != estadoAnterior) {
    // Publica imediatamente a mudança
    client.publish("empresa/estoque/porta/status", estadoAtual == HIGH ? "1" : "0");
    
    if (estadoAtual == HIGH) {
      Serial.println("Porta ABERTA - Iniciando cronômetro.");
      tempoAbertura = millis(); // "Zera" o cronômetro salvando o tempo atual
      alertaTempoExcedidoEnviado = false;
    } else {
      Serial.println("Porta FECHADA - Cronômetro parado.");
      tempoAbertura = 0; // Reseta o tempo
    }
    estadoAnterior = estadoAtual;
  }

  // --- Lógica 2: Alerta de Tempo Excedido (Uso do millis para contagem) ---
  if (estadoAtual == HIGH && !alertaTempoExcedidoEnviado) {
    // Compara o tempo atual com o momento em que a porta abriu
    if (millis() - tempoAbertura >= 60000) { 
      client.publish("empresa/estoque/sistema/alerta", "ALERTA: PORTA ABERTA HÁ MAIS DE 1 MINUTO");
      Serial.println("!!! CRÍTICO: TEMPO EXCEDIDO !!!");
      alertaTempoExcedidoEnviado = true; // Garante que o alerta seja enviado apenas uma vez por abertura
    }
  }
}
