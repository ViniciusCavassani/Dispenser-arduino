
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <WiFi.h>
#include <time.h>
#include <ESP32Servo.h>

RTC_DS1307 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

// =====================================================
// CONFIGURACAO WI-FI E NTP
// =====================================================

const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_SENHA = "SUA_SENHA";

const long FUSO_HORARIO_SEGUNDOS = -3L * 3600L;
const int HORARIO_VERAO_SEGUNDOS = 0;

const char* SERVIDOR_NTP_1 = "br.pool.ntp.org";
const char* SERVIDOR_NTP_2 = "time.google.com";

// =====================================================
// OBJETOS
// =====================================================

// =====================================================
// PINOS
// =====================================================

const byte SERVO_PIN = 25;

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
// CONFIGURACOES
// =====================================================

const unsigned long DURACAO_ALERTA = 30000UL;
const unsigned long INTERVALO_LCD = 500UL;
const unsigned long DURACAO_MENSAGEM = 1800UL;

const int ANGULO_INICIAL = 0;
const int ANGULO_ALERTA = 90;

// =====================================================
// VARIAVEIS
// =====================================================

// Entrada do horario
char horario[5] = "";
byte qtdDigitos = 0;

// Alarme diario
int horaRemedio = -1;
int minutoRemedio = -1;

// Relogio
bool horaValidaRTC = false;

// Alerta e servo
bool alertaAtivo = false;
unsigned long inicioAlerta = 0;

// Evita repetir o alarme no mesmo dia
// durante a execucao normal do programa.
int64_t ultimoDiaAlertado = -1;

// Atualizacao do LCD
unsigned long ultimoUpdateLCD = 0;

// Mensagens temporarias
char mensagem1[17] = "";
char mensagem2[17] = "";
unsigned long mensagemAte = 0;

// =====================================================
// PROTOTIPOS
// =====================================================

bool sincronizarRTCComInternet();
bool dataRTCValida(const DateTime& data);

void lerTeclado();
void confirmarHorario();
void verificarHorario();
void controlarAlerta();
void atualizarLCD();
void mostrarMensagem(const char* linha1, const char* linha2);
void imprimirDoisDigitosLCD(int valor);

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(115200);

  // Inicializa I2C do ESP32
  Wire.begin(21, 22);

  // Inicializa LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Dispenser");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando");

  // Inicializa servo
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(ANGULO_INICIAL);

  // Inicializa RTC
  if (!rtc.begin()) {
    Serial.println("ERRO: RTC nao encontrado!");

    mostrarMensagem("ERRO NO RTC", "Verifique fios");

    while (true) {
      delay(100);
    }
  }

  // Tenta sincronizar a hora pela internet
  horaValidaRTC = sincronizarRTCComInternet();

  // Se o NTP falhar, tenta usar o RTC
  if (!horaValidaRTC) {
    DateTime guardada = rtc.now();

    if (rtc.isrunning() && dataRTCValida(guardada)) {
      horaValidaRTC = true;

      Serial.println("Usando horario guardado no RTC.");
    } else {
      Serial.println("ERRO: nao existe hora valida.");

      mostrarMensagem("SEM HORA VALIDA", "Confira WiFi/RTC");
    }
  }

  if (horaValidaRTC) {
    DateTime agora = rtc.now();

    Serial.printf(
      "Data: %02d/%02d/%04d\n",
      agora.day(), agora.month(), agora.year()
    );

    Serial.printf(
      "Hora: %02d:%02d:%02d\n",
      agora.hour(), agora.minute(), agora.second()
    );
  }

  Serial.println();
  Serial.println("DISPENSER DE REMEDIO");
  Serial.println("Digite HHMM e pressione #.");
  Serial.println("Pressione * para apagar.");
  Serial.println("O alarme sera diario.");

  mostrarMensagem("Digite HHMM", "Confirme com #");
}

// =====================================================
// LOOP PRINCIPAL
// =====================================================

void loop() {
  lerTeclado();
  verificarHorario();
  controlarAlerta();
  atualizarLCD();
}

// =====================================================
// SINCRONIZACAO NTP
// =====================================================

bool sincronizarRTCComInternet() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");

  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - inicio < 20000UL) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Falha na conexao Wi-Fi.");

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    return false;
  }

  Serial.println("Wi-Fi conectado.");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Buscando hora");

  configTime(
    FUSO_HORARIO_SEGUNDOS,
    HORARIO_VERAO_SEGUNDOS,
    SERVIDOR_NTP_1,
    SERVIDOR_NTP_2
  );

  struct tm horaInternet;

  bool recebeuHora = getLocalTime(&horaInternet, 15000);

  if (recebeuHora &&
      horaInternet.tm_year + 1900 >= 2024) {

    DateTime novaHora(
      horaInternet.tm_year + 1900,
      horaInternet.tm_mon + 1,
      horaInternet.tm_mday,
      horaInternet.tm_hour,
      horaInternet.tm_min,
      horaInternet.tm_sec
    );

    rtc.adjust(novaHora);

    Serial.println("RTC sincronizado com NTP.");

    Serial.printf(
      "Hora recebida: %02d:%02d:%02d\n",
      novaHora.hour(),
      novaHora.minute(),
      novaHora.second()
    );

    mostrarMensagem("Hora sincronizada", "RTC atualizado");

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    return true;
  }

  Serial.println("Falha ao obter hora NTP.");

  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  return false;
}

// =====================================================
// VALIDACAO DO RELOGIO
// =====================================================

bool dataRTCValida(const DateTime& data) {
  return data.year() >= 2024 &&
         data.year() <= 2099 &&
         data.month() >= 1 &&
         data.month() <= 12 &&
         data.day() >= 1 &&
         data.day() <= 31;
}

// =====================================================
// TECLADO
// =====================================================

void lerTeclado() {
  if (alertaAtivo) {
    return;
  }

  char tecla = keypad.getKey();

  if (!tecla) {
    return;
  }

  Serial.print("Tecla pressionada: ");
  Serial.println(tecla);

  // Digitar numeros
  if (tecla >= '0' && tecla <= '9') {
    if (qtdDigitos < 4) {
      horario[qtdDigitos] = tecla;
      qtdDigitos++;
      horario[qtdDigitos] = '\0';

      mensagemAte = 0;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Horario HHMM:");
      lcd.setCursor(0, 1);
      lcd.print(horario);
    }
  }

  // Apagar digitacao
  else if (tecla == '*') {
    qtdDigitos = 0;
    horario[0] = '\0';

    mensagemAte = 0;

    mostrarMensagem("Digite HHMM", "Confirme com #");

    Serial.println("Digitacao apagada.");
  }

  // Confirmar horario
  else if (tecla == '#') {
    confirmarHorario();
  }
}

// =====================================================
// CONFIRMAR ALARME
// =====================================================

void confirmarHorario() {
  if (qtdDigitos != 4) {
    mostrarMensagem("Digite 4 numeros", "Ex.: 2030");

    Serial.println("Horario incompleto.");
    return;
  }

  int hora = (horario[0] - '0') * 10 +
             (horario[1] - '0');

  int minuto = (horario[2] - '0') * 10 +
               (horario[3] - '0');

  // Limpa a digitacao
  qtdDigitos = 0;
  horario[0] = '\0';

  if (hora > 23 || minuto > 59) {
    mostrarMensagem("HORARIO INVALIDO", "Tente novamente");

    Serial.println("Horario invalido.");
    return;
  }

  if (!horaValidaRTC) {
    mostrarMensagem("SEM HORA VALIDA", "Confira WiFi/RTC");

    Serial.println("Alarme nao programado: RTC invalido.");
    return;
  }

  horaRemedio = hora;
  minutoRemedio = minuto;

  // Permite programar um novo alarme
  ultimoDiaAlertado = -1;

  Serial.printf(
    "Alarme diario programado: %02d:%02d\n",
    horaRemedio, minutoRemedio
  );

  char linha[17];

  snprintf(
    linha,
    sizeof(linha),
    "Horario: %02d:%02d",
    horaRemedio,
    minutoRemedio
  );

  mostrarMensagem("Alarme diario", linha);
}

// =====================================================
// VERIFICAR HORARIO
// =====================================================

void verificarHorario() {
  if (!horaValidaRTC ||
      horaRemedio < 0 ||
      minutoRemedio < 0 ||
      alertaAtivo) {
    return;
  }

  DateTime agora = rtc.now();

  int64_t diaAtual = agora.unixtime() / 86400LL;

  // Aciona em qualquer segundo do minuto programado,
  // mas somente uma vez por dia durante a execucao.
  if (agora.hour() == horaRemedio &&
      agora.minute() == minutoRemedio &&
      diaAtual != ultimoDiaAlertado) {

    ultimoDiaAlertado = diaAtual;

    alertaAtivo = true;
    inicioAlerta = millis();

    servo.write(ANGULO_ALERTA);

    Serial.println("========================");
    Serial.println("ALERTA: TOMAR REMEDIO!");
    Serial.printf(
      "Horario: %02d:%02d\n",
      agora.hour(), agora.minute()
    );
    Serial.println("========================");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TOMAR REMEDIO");
    lcd.setCursor(0, 1);
    lcd.print("Hora: ");

    imprimirDoisDigitosLCD(agora.hour());
    lcd.print(":");
    imprimirDoisDigitosLCD(agora.minute());
  }
}

// =====================================================
// CONTROLAR ALERTA
// =====================================================

void controlarAlerta() {
  if (!alertaAtivo) {
    return;
  }

  if (millis() - inicioAlerta >= DURACAO_ALERTA) {
    servo.write(ANGULO_INICIAL);

    alertaAtivo = false;

    Serial.println("Alerta finalizado.");
    Serial.println("Servo retornou a 0 graus.");

    // O alarme continua programado para os proximos dias.
    mostrarMensagem("Alerta finalizado", "Alarme diario");
  }
}

// =====================================================
// MENSAGENS TEMPORARIAS
// =====================================================

void mostrarMensagem(const char* linha1, const char* linha2) {
  snprintf(mensagem1, sizeof(mensagem1), "%-16.16s", linha1);
  snprintf(mensagem2, sizeof(mensagem2), "%-16.16s", linha2);

  mensagemAte = millis() + DURACAO_MENSAGEM;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(mensagem1);
  lcd.setCursor(0, 1);
  lcd.print(mensagem2);
}

// =====================================================
// ATUALIZAR LCD
// =====================================================

void atualizarLCD() {
  if (alertaAtivo) {
    return;
  }

  unsigned long agoraMs = millis();

  if (agoraMs - ultimoUpdateLCD < INTERVALO_LCD) {
    return;
  }

  ultimoUpdateLCD = agoraMs;

  // Mostra mensagens temporarias
  if (mensagemAte != 0 &&
      (int32_t)(mensagemAte - agoraMs) > 0) {
    return;
  }

  mensagemAte = 0;

  if (!horaValidaRTC) {
    lcd.setCursor(0, 0);
    lcd.print("SEM HORA VALIDA ");
    lcd.setCursor(0, 1);
    lcd.print("Confira WiFi/RTC");
    return;
  }

  // Durante a digitacao, nao sobrescreve o horario
  if (qtdDigitos > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Horario HHMM:   ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(horario);
    return;
  }

  DateTime agora = rtc.now();

  // Linha 1: data
  lcd.setCursor(0, 0);
lcd.print("                ");
lcd.setCursor(0, 0);

  imprimirDoisDigitosLCD(agora.day());
  lcd.print("/");
  imprimirDoisDigitosLCD(agora.month());
  lcd.print("/");
  lcd.print(agora.year());

// Linha 2: hora
lcd.setCursor(0, 1);
lcd.print("                ");
lcd.setCursor(0, 1);

  imprimirDoisDigitosLCD(agora.hour());
  lcd.print(":");
  imprimirDoisDigitosLCD(agora.minute());
  lcd.print(":");
  imprimirDoisDigitosLCD(agora.second());
  lcd.print("    ");
}

// =====================================================
// IMPRIMIR DOIS DIGITOS
// =====================================================

void imprimirDoisDigitosLCD(int valor) {
  if (valor < 10) {
    lcd.print("0");
  }

  lcd.print(valor);
}
