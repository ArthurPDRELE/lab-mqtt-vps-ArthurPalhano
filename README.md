Escopo do Projeto — Arthur Palhano
Integrantes

    Arthur Palhano do Rosario — Técnico em Eletrônica

Problema

O projeto aborda a vulnerabilidade de segurança em áreas de estoque restrito, onde o acesso não supervisionado ou o esquecimento de portas abertas gera riscos de perdas. O sistema garante rastreabilidade total e alertas em tempo real para portas deixadas abertas.
O que será monitorado

Estado binário (Aberto/Fechado) de uma porta de acesso e o tempo de permanência em estado aberto.
Sensor / Dado

Sensor Magnético de Contato (Reed Switch) conectado ao ESP32, operando por evento (mudança de estado).
Estrutura de Tópicos MQTT
Tópico	Descrição	Publisher	Subscriber
empresa/estoque/porta/status	Envia Porta fechada (fechado) ou Porta aberta (aberto)	ESP32	Terminal VPS
empresa/estoque/sistema/alerta	Envia ALERTA CRITICO PORTA ABERTA (tempo excedido > 60s)	ESP32	Terminal VPS
Resultado Esperado

Monitoramento via logs em tempo real no terminal confirmando cada abertura e fechamento, além de um aviso visual crítico caso a porta permaneça aberta por mais de um minuto.
Hardware / Software planejado

    Publisher: ESP32 (Simulado no Wokwi)

    Broker: Mosquitto na VPS DigitalOcean

    Subscriber: Terminal SSH da VPS (via mosquitto_sub)
