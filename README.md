# Dispenser Inteligente de Medicamentos 💊

Projeto desenvolvido com **ESP32**, com o objetivo de criar um sistema programável de lembrete de medicamentos. O usuário configura um horário pelo teclado matricial, acompanha a data e a hora no display LCD e recebe um alerta quando chega o momento programado.

O repositório reúne duas implementações do projeto: uma versão com alerta visual por LED e outra versão expandida com sincronização de horário pela internet e acionamento de servo motor.

> **Aviso:** este projeto é um protótipo educacional. Ele não substitui orientação médica, não confirma a ingestão do medicamento e não deve ser utilizado como único meio de controle de tratamentos.

## ✨ Funcionalidades

### Versão com LED — `dispenser.ino`
- Exibe data e hora em um display LCD 16x2.
- Permite cadastrar um horário no formato `HHMM` pelo teclado 4x4.
- Usa a tecla `#` para confirmar o horário.
- Usa a tecla `*` para apagar a digitação.
- Valida horas entre 00 e 23 e minutos entre 00 e 59.
- Aciona um LED por aproximadamente 30 segundos no horário programado.

### Versão com Wi-Fi e servo — `DispenserArduino.ino`
- Conecta-se ao Wi-Fi para obter a hora por servidores NTP.
- Atualiza o módulo RTC DS1307 com o horário recebido pela internet.
- Continua tentando utilizar a hora armazenada no RTC caso a sincronização falhe.
- Permite programar um alarme diário pelo teclado.
- Aciona um servo motor quando chega o horário configurado.
- Mantém o alerta ativo por aproximadamente 30 segundos e depois retorna o servo à posição inicial.

## 🧰 Componentes utilizados

| Componente | Função |
|---|---|
| ESP32 DevKit | Unidade de controle do sistema |
| RTC DS1307 | Mantém data e hora |
| LCD 16x2 com I2C | Exibe informações e mensagens |
| Teclado matricial 4x4 | Entrada do horário do lembrete |
| LED e resistor | Alerta visual da versão básica |
| Servo motor | Acionamento mecânico da versão avançada |

## 🔌 Ligações e pinagem

### Conexões comuns

| Componente | ESP32 |
|---|---|
| SDA (RTC e LCD) | GPIO 21 |
| SCL (RTC e LCD) | GPIO 22 |
| Teclado — linhas | GPIO 23, 19, 18 e 5 |
| Teclado — colunas | GPIO 17, 16, 4 e 2 |

O endereço I2C configurado para o LCD é `0x27`.

### Versão com LED

| Componente | Pino |
|---|---|
| LED | GPIO 13 |

### Versão com servo motor

| Componente | Pino |
|---|---|
| Sinal do servo | GPIO 25 |

**Atenção à alimentação:** verifique a tensão e a corrente exigidas pelo servo motor. Se necessário, utilize uma fonte externa adequada e conecte o GND da fonte ao GND do ESP32. Não alimente um servo que exija corrente elevada diretamente pelo pino de 3,3 V da placa.

## ⌨️ Como utilizar

1. Ligue o ESP32 e aguarde a inicialização do RTC e do display.
2. Digite quatro números no teclado, no formato `HHMM`.
   - Exemplo: `2030` representa 20:30.
3. Pressione `#` para confirmar.
4. Se precisar corrigir a digitação, pressione `*` para limpar.
5. Quando o horário programado chegar, o sistema inicia o alerta.
6. Na implementação com LED, o LED permanece aceso por cerca de 30 segundos. Na implementação com servo, o motor vai à posição configurada e retorna após o período do alerta.

O alarme é diário enquanto o programa permanece em execução. Na versão com LED, o horário é apagado ao final do alerta, permitindo cadastrar outro. Na versão com servo, o horário permanece programado para os dias seguintes.

## 🕒 Relógio e sincronização

A versão básica utiliza o horário mantido pelo DS1307. Para ajustar o relógio inicialmente, o código contém um exemplo de `rtc.adjust()` usando a data e a hora da compilação. Essa linha deve ser usada apenas para o ajuste inicial e depois comentada, conforme as instruções no próprio código.

A versão avançada tenta sincronizar o relógio com servidores NTP pela rede Wi-Fi. Se não conseguir conexão, tenta utilizar a hora já armazenada no RTC.

## 📚 Bibliotecas

Instale as bibliotecas correspondentes à versão que deseja compilar:

- **RTClib** — comunicação com o módulo RTC DS1307.
- **LiquidCrystal_I2C** — controle do display LCD via I2C.
- **Keypad** — leitura do teclado matricial.
- **ESP32Servo** — controle do servo motor (versão avançada).

As bibliotecas `WiFi.h` e `time.h` são utilizadas pela plataforma ESP32 para conexão e sincronização NTP.

## ⚙️ Configuração do Wi-Fi

Na versão avançada, abra `DispenserArduino.ino` e informe suas credenciais localmente nas constantes `WIFI_SSID` e `WIFI_SENHA`.

**Não publique sua senha no GitHub.** Antes de enviar alterações, confira se as credenciais foram removidas do código e do histórico público. Se uma senha já foi publicada, altere-a.

## 📁 Arquivos do repositório

| Arquivo | Descrição |
|---|---|
| `dispenser.ino` | Implementação com LED e programação de horário |
| `DispenserArduino.ino` | Implementação com Wi-Fi/NTP e servo motor |
| `README.md` | Documentação, componentes, pinagem e instruções |

## 🎓 Conhecimentos aplicados

- Programação embarcada em C/C++.
- Desenvolvimento com ESP32.
- Comunicação I2C.
- Integração de sensores, módulos e periféricos.
- Leitura de teclado matricial.
- Controle de saídas digitais e servo motor.
- Utilização de RTC e sincronização NTP.
- Depuração pelo monitor serial.

## 🔭 Possíveis melhorias

- Permitir cadastrar mais de um horário de lembrete.
- Salvar os horários na memória não volátil do ESP32.
- Adicionar confirmação de retirada do medicamento.
- Desenvolver uma interface para configuração via Wi-Fi.
- Criar um mecanismo físico de dispensação com compartimentos.

---

Desenvolvido por [Vinicius Cavassani](https://github.com/ViniciusCavassani) como projeto de estudo em eletrônica e sistemas embarcados.
