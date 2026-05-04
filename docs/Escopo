# Monitoramento e Automação de Estoque Inteligente

## Integrante
* **Arthur Palhano do Rosario** — Técnico em Eletrônica

## Problema
O projeto aborda a vulnerabilidade de segurança e a ineficiência operacional em áreas de estoque restrito. O objetivo é mitigar riscos vindos de acessos não supervisionados, portas esquecidas abertas e o desperdício de energia elétrica.

## O que será monitorado e controlado
1. **Acesso Físico:** Estado (Aberto/Fechado) da porta e tempo de permanência.
2. **Presença Real:** Detecção de movimento infravermelho no ambiente.
3. **Ambiente:** Medição de luminosidade para validar a necessidade de luz artificial.
4. **Atuação Automática:** Controle de desligamento da iluminação após 10 minutos de inatividade.

## Sensores, Atuadores e Dados
* **Sensor Magnético (Reed Switch):** Monitora a abertura da porta.
* **Sensor PIR (HC-SR501):** Detecta movimento para segurança e automação.
* **Sensor LDR:** Mede a luminosidade ambiente.
* **Módulo Relé:** Atua no desligamento da carga (lâmpada) por comando do ESP32.

## Estrutura de Tópicos MQTT
O sistema utiliza uma hierarquia organizada para facilitar a filtragem de dados na VPS:

| Tópico | Descrição | Mensagens de Exemplo |
| :--- | :--- | :--- |
| `empresa/estoque/porta/status` | Estado físico da porta | `aberto` / `fechado` |
| `empresa/estoque/presenca/alerta` | Notificação imediata de movimento | `ALERTA: Movimento detectado!` |
| `empresa/estoque/luz/nivel` | Leitura da luminosidade (0-4095) | `1250` |
| `empresa/estoque/luz/automacao` | Status do desligamento automático | `Luzes desligadas por inatividade (10 min).` |
| `empresa/estoque/sistema/alerta` | Alertas críticos do sistema | `ALERTA CRITICO: PORTA ABERTA > 60s` |

## Lógica de Funcionamento e Resultados Esperados
* **Segurança Ativa:** O sistema envia uma notificação instantânea ao detectar movimento, permitindo que a equipe de segurança receba o alerta em tempo real via broker.
* **Gestão de Energia:** Se o sensor PIR não detectar movimento por um período ininterrupto de 10 minutos, o ESP32 desliga automaticamente a iluminação e publica a ação no tópico de automação.
* **Rastreabilidade:** Monitoramento via logs em tempo real no terminal SSH, confirmando eventos de entrada, presença e economia de energia.
* **Auditoria:** Os dados coletados permitem analisar horários de maior fluxo e identificar incidentes de esquecimento de luzes acesas.

## Hardware / Software Planejado
* **Publisher:** ESP32 (Simulação via Wokwi).
* **Broker:** Mosquitto MQTT configurado em VPS DigitalOcean.
* **Subscriber:** Terminal SSH da VPS (via `mosquitto_sub`) para visualização de logs.
