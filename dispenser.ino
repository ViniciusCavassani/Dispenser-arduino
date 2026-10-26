#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>

// =====================================================
// OBJETOS
// =====================================================

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================================================
// LED
// =====================================================

#define LED 13

// =====================================================
// KEYPAD 4x4
// =====================================================

const byte LINHAS = 4;
const byte COLUNAS = 4;

char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinosLinhas[LINHAS] = {23, 19, 18, 5};
byte pinosColunas[COLUNAS] = {17, 16, 4, 2};

Keypad keypad = Keypad(
  makeKeymap(teclas),
  pinosLinhas,
  pinosColunas,
  LINHAS,
  COLUNAS
);

// =====================================================
// VARIÁVEIS
// =====================================================

String horario = "";

unsigned long ultimoUpdateLCD = 0;
unsigned long inicioAlerta = 0;

int horaRemedio = -1;
int minutoRemedio = -1;

bool alertaAtivo = false;
bool alertaJaExecutado = false;

// =====================================================
// SETUP
// =====================================================

void setup() {

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // I2C do ESP32
  Wire.begin(21, 22);

  // LCD
  lcd.init();
  lcd.backlight();

  // Serial
  Serial.begin(115200);

  // RTC
  if (!rtc.begin()) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC nao");
    lcd.setCursor(0, 1);
    lcd.print("encontrado!");

    Serial.println("RTC nao encontrado!");

    while (1);
  }

  // =================================================
  // CONFIGURAR O RTC
  // =================================================
  // USE ESTA LINHA SOMENTE NO PRIMEIRO UPLOAD.
  // Depois de configurar o RTC, COMENTE a linha
  // e faça outro upload.
  // =================================================

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Mensagem inicial
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Digite HHMM");

  Serial.println("================================");
  Serial.println("DISPENSER DE REMEDIO");
  Serial.println("================================");
  Serial.println("Digite o horario no formato HHMM");
  Serial.println("Exemplo: 2030");
  Serial.println("Use # para confirmar");
  Serial.println("Use * para apagar");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  lerTeclado();

  verificarHorario();

  controlarAlerta();

  atualizarLCD();
}

// =====================================================
// TECLADO
// =====================================================

void lerTeclado() {

  // Durante o alerta, não permite cadastrar outro horário
  if (alertaAtivo) {
    return;
  }

  char tecla = keypad.getKey();

  if (tecla) {

    Serial.print("Tecla: ");
    Serial.println(tecla);

    // -----------------------------------------------
    // NÚMEROS
    // -----------------------------------------------

    if (isDigit(tecla)) {

      if (horario.length() < 4) {

        horario += tecla;

        Serial.print("Digitando: ");
        Serial.println(horario);
      }
    }

    // -----------------------------------------------
    // CONFIRMAR COM #
    // -----------------------------------------------

    if (tecla == '#') {

      Serial.print("Horario confirmado: ");
      Serial.println(horario);

      if (horario.length() == 4) {

        horaRemedio = horario.substring(0, 2).toInt();
        minutoRemedio = horario.substring(2, 4).toInt();

        // Verifica se o horário é válido
        if (horaRemedio >= 0 && horaRemedio <= 23 &&
            minutoRemedio >= 0 && minutoRemedio <= 59) {

          Serial.print("Horario salvo: ");

          if (horaRemedio < 10) {
            Serial.print("0");
          }

          Serial.print(horaRemedio);
          Serial.print(":");

          if (minutoRemedio < 10) {
            Serial.print("0");
          }

          Serial.println(minutoRemedio);

          // Permite que o alerta aconteça nesse horário
          alertaJaExecutado = false;

        } else {

          Serial.println("Horario invalido!");

          horaRemedio = -1;
          minutoRemedio = -1;
        }

      } else {

        Serial.println("Digite exatamente 4 numeros!");

        horaRemedio = -1;
        minutoRemedio = -1;
      }

      horario = "";
    }

    // -----------------------------------------------
    // LIMPAR COM *
    // -----------------------------------------------

    if (tecla == '*') {

      horario = "";

      Serial.println("Horario apagado");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Digite HHMM");
    }
  }
}

// =====================================================
// VERIFICAR HORÁRIO
// =====================================================

void verificarHorario() {

  if (horaRemedio < 0 || alertaAtivo) {
    return;
  }

  DateTime now = rtc.now();

  if (now.hour() == horaRemedio &&
      now.minute() == minutoRemedio &&
      !alertaJaExecutado) {

    // Ativa o alerta
    alertaAtivo = true;

    // Guarda o momento em que o alerta começou
    inicioAlerta = millis();

    // Evita repetir o alerta no mesmo minuto
    alertaJaExecutado = true;

    // Liga LED
    digitalWrite(LED, HIGH);

    // Serial
    Serial.println("================================");
    Serial.println("TOMAR REMEDIO");
    Serial.println("================================");

    // LCD
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("TOMAR REMEDIO");

    lcd.setCursor(0, 1);
    lcd.print("Hora: ");

    if (horaRemedio < 10) {
      lcd.print("0");
    }

    lcd.print(horaRemedio);
    lcd.print(":");

    if (minutoRemedio < 10) {
      lcd.print("0");
    }

    lcd.print(minutoRemedio);
  }
}

// =====================================================
// CONTROLAR ALERTA
// =====================================================

void controlarAlerta() {

  if (!alertaAtivo) {
    return;
  }

  // Alerta dura 30 segundos
  if (millis() - inicioAlerta >= 30000) {

    // Desliga LED
    digitalWrite(LED, LOW);

    // Desativa alerta
    alertaAtivo = false;

    // Apaga horário antigo
    horaRemedio = -1;
    minutoRemedio = -1;

    // Limpa horário digitado
    horario = "";

    // LCD volta para cadastro
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Digite HHMM");

    // Serial
    Serial.println("Alerta encerrado.");
    Serial.println("Digite um novo horario HHMM");
  }
}

// =====================================================
// ATUALIZAR LCD
// =====================================================

void atualizarLCD() {

  // Enquanto o alerta estiver ativo,
  // não sobrescreve o LCD
  if (alertaAtivo) {
    return;
  }

  // Atualiza o LCD a cada 1 segundo
  if (millis() - ultimoUpdateLCD >= 1000) {

    ultimoUpdateLCD = millis();

    DateTime now = rtc.now();

    // -----------------------------------------------
    // DATA
    // -----------------------------------------------

    lcd.clear();

    lcd.setCursor(0, 0);

    if (now.day() < 10) {
      lcd.print("0");
    }

    lcd.print(now.day());
    lcd.print("/");

    if (now.month() < 10) {
      lcd.print("0");
    }

    lcd.print(now.month());
    lcd.print("/");

    lcd.print(now.year());

    // -----------------------------------------------
    // HORA
    // -----------------------------------------------

    lcd.setCursor(0, 1);

    if (now.hour() < 10) {
      lcd.print("0");
    }

    lcd.print(now.hour());
    lcd.print(":");

    if (now.minute() < 10) {
      lcd.print("0");
    }

    lcd.print(now.minute());
    lcd.print(":");

    if (now.second() < 10) {
      lcd.print("0");
    }

    lcd.print(now.second());
  }
}
