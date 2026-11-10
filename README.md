# Dispenser de medicamentos com ESP32

Protótipo educacional de lembrete de medicamentos. O horário é digitado em um teclado 4x4, e o relógio e as mensagens aparecem em um LCD 16x2. O repositório contém duas versões independentes.

> Este projeto é apenas um protótipo. Ele não confirma a ingestão do medicamento e não deve ser usado como único meio de controle de tratamentos.

## Versões

### Versão com LED — `dispenser.ino`

Usa o RTC DS1307 para manter data e hora. Digite o horário no formato `HHMM` e pressione `#` para confirmar. Pressione `*` para limpar a digitação. No horário programado, o LED no GPIO 13 acende por cerca de 30 segundos; depois, o horário é apagado e pode ser cadastrado outro.

### Versão com Wi-Fi, NTP e servo — `DispenserArduino.ino`

Tenta buscar a hora pela internet e atualizar o RTC DS1307. Se a conexão falhar, tenta usar a hora guardada no RTC. O alarme é diário enquanto o programa estiver ligado. No horário programado, o servo no GPIO 25 vai à posição de alerta por cerca de 30 segundos e retorna à posição inicial.

Os horários ficam apenas na memória do programa e são perdidos quando o ESP32 reinicia.

## Componentes

| Componente | Uso |
|---|---|
| ESP32 DevKit | Controlador |
| RTC DS1307 | Relógio |
| LCD 16x2 I2C | Data, hora e mensagens |
| Teclado matricial 4x4 | Cadastro do horário |
| LED e resistor | Alerta da versão LED |
| Servo motor | Acionamento da versão com servo |

## Ligações

| Sinal | GPIO do ESP32 |
|---|---|
| SDA do RTC e LCD | 21 |
| SCL do RTC e LCD | 22 |
| Linhas do teclado | 23, 19, 18, 5 |
| Colunas do teclado | 17, 16, 4, 2 |
| LED (versão LED) | 13 |
| Sinal do servo (versão servo) | 25 |

O LCD está configurado no endereço I2C `0x27`. Verifique o endereço do seu módulo. O servo pode exigir uma fonte externa apropriada; nesse caso, conecte o GND da fonte ao GND do ESP32. Não alimente o servo pelo pino de 3,3 V.

## Bibliotecas

- RTClib
- LiquidCrystal_I2C
- Keypad
- ESP32Servo (somente na versão com servo)

Wi-Fi e sincronização de horário usam bibliotecas da plataforma ESP32.

## Como abrir e enviar

1. Baixe ou clone o repositório.
2. Na Arduino IDE, abra **somente um** dos arquivos `.ino` da versão que deseja usar.
3. Instale as bibliotecas correspondentes.
4. Se usar a versão com Wi-Fi, informe SSID e senha localmente em `WIFI_SSID` e `WIFI_SENHA`. Nunca publique a senha no GitHub.
5. Selecione uma placa ESP32 compatível e a porta correta.
6. Confira a pinagem e envie o programa.

Na versão com LED, o RTC pode ser ajustado inicialmente pela linha comentada `rtc.adjust()` no código; depois do ajuste, deixe-a comentada para os próximos envios.

## O que o projeto demonstra

- Programação embarcada em C/C++.
- Integração de ESP32, RTC, LCD, teclado, LED e servo.
- Comunicação I2C.
- Configuração de Wi-Fi e sincronização de hora por NTP.
- Depuração pelo Monitor Serial.

## Melhorias possíveis

- Salvar o horário na memória não volátil para preservar a configuração após reiniciar.
- Permitir mais de um lembrete.
- Criar uma interface para cadastrar os horários.
- Desenvolver um mecanismo físico com compartimentos e validar sua operação.
