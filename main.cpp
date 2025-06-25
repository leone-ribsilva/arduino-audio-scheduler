#include <SoftwareSerial.h>              // Biblioteca para comunicacao serial com o DFPlayer via pinos digitais
#include <DFRobotDFPlayerMini.h>         // Biblioteca especifica para controle do modulo DFPlayer Mini
#include <Wire.h>                        // Biblioteca para comunicacao I2C (necessaria para o RTC)
#include <RTClib.h>                      // Biblioteca para o modulo RTC DS3231
#include <SPI.h>                         // Comunicacao SPI (necessaria para o cartao SD)
#include <SD.h>                          // Biblioteca para manipular arquivos no cartao SD

RTC_DS3231 rtc;                          // Cria o objeto do modulo RTC
SoftwareSerial mySerial(10, 11);        // Define os pinos RX e TX para comunicacao com o DFPlayer
DFRobotDFPlayerMini player;             // Cria o objeto que controla o DFPlayer

#define SD_CS_PIN 4                     // Define o pino CS do leitor de cartao SD

const int MAX_SCHEDULES = 10;           // Maximo de agendamentos que podem ser lidos do arquivo
const int MAX_AUDIO_ITEMS = 5;          // Maximo de audios por agendamento

// Estrutura para um item de audio (numero do arquivo e repeticoes)
struct AudioItem {
  int fileNumber;                       // Ex: 1 representa "001.mp3"
  int repeatCount;                      // Quantas vezes esse audio deve tocar
};

// Estrutura para um agendamento completo
struct AudioSchedule {
  int hour;                             // Hora da execucao
  int minute;                           // Minuto da execucao
  int days[7];                          // Lista de dias da semana permitidos (0=Dom, 6=Sab)
  int dayCount;                         // Quantos dias foram definidos
  AudioItem audioList[MAX_AUDIO_ITEMS]; // Lista de audios a tocar nesse horario
  int audioCount;                       // Quantos audios ha na lista
};

AudioSchedule schedules[MAX_SCHEDULES]; // Array de agendamentos
int scheduleCount = 0;                  // Quantos agendamentos foram carregados
bool alreadyPlayed[MAX_SCHEDULES];      // Controle para nao tocar duas vezes no mesmo minuto

void setup() {
  Serial.begin(9600);                   // Inicia comunicacao com o monitor serial (debug)
  mySerial.begin(9600);                 // Inicia comunicacao com o DFPlayer

  if (!rtc.begin()) {                   // Verifica se o RTC esta conectado
    Serial.println("RTC erro.");
    while (1);                          // Para tudo se falhar
  }

  if (!player.begin(mySerial)) {        // Verifica se o DFPlayer esta respondendo
    Serial.println("DFPlayer erro.");
    while (1);
  }

  player.volume(25);                    // Define o volume entre 0 e 30

  if (!SD.begin(SD_CS_PIN)) {           // Tenta inicializar o cartao SD
    Serial.println("Erro ao iniciar cartao SD.");
    while (1);
  }

  lerConfiguracoesDoSD();               // Le o arquivo config.txt e popula os agendamentos

  for (int i = 0; i < MAX_SCHEDULES; i++) 
    alreadyPlayed[i] = false;           // Inicializa as flags de execucao
}

void loop() {
  DateTime now = rtc.now();             // Pega a data e hora atual
  int wd = now.dayOfTheWeek();          // Pega o dia da semana (0=domingo)

  for (int i = 0; i < scheduleCount; i++) {
    if (!isDayAllowed(schedules[i], wd)) continue; // Verifica se hoje e permitido

    if (now.hour() == schedules[i].hour &&
        now.minute() == schedules[i].minute &&
        !alreadyPlayed[i]) {

      // Percorre todos os audios programados para este horario
      for (int j = 0; j < schedules[i].audioCount; j++) {
        AudioItem item = schedules[i].audioList[j];
        for (int r = 0; r < item.repeatCount; r++) {
          tocarAudio(item.fileNumber); // Toca o audio conforme numero e repeticoes
        }
      }

      alreadyPlayed[i] = true; // Marca que esse agendamento ja foi executado neste minuto
    }

    if (now.minute() != schedules[i].minute) {
      alreadyPlayed[i] = false; // Libera novamente para tocar se o minuto for diferente
    }
  }

  delay(1000); // Aguarda 1 segundo antes de repetir a verificacao
}

// Verifica se o dia da semana atual esta permitido para o agendamento
bool isDayAllowed(const AudioSchedule& s, int wd) {
  for (int i = 0; i < s.dayCount; i++) {
    if (s.days[i] == wd) return true;
  }
  return false;
}

// Funcao para tocar um audio e aguardar seu termino
void tocarAudio(int num) {
  player.play(num); // Solicita ao DFPlayer que toque o arquivo

  while (true) {
    if (player.available() && player.readType() == DFPlayerPlayFinished)
      break; // Sai quando o audio terminar
    delay(100);
  }
}

// Funcao para ler o arquivo config.txt do cartao SD e carregar os agendamentos
void lerConfiguracoesDoSD() {
  File file = SD.open("/config.txt"); // Abre o arquivo config.txt na raiz do cartao
  if (!file) {
    Serial.println("Arquivo config.txt nao encontrado.");
    return;
  }

  while (file.available() && scheduleCount < MAX_SCHEDULES) {
    String linha = file.readStringUntil('\n'); // Le uma linha do arquivo
    linha.trim(); // Remove espacos e quebras de linha

    if (linha.length() == 0 || linha.startsWith("#")) continue; // Ignora linhas vazias ou comentarios

    AudioSchedule sched;
    sched.dayCount = 0;
    sched.audioCount = 0;

    int sep1 = linha.indexOf(';');
    int sep2 = linha.indexOf(';', sep1 + 1);
    if (sep1 == -1 || sep2 == -1) continue; // Valida se a linha esta bem formada

    String horario = linha.substring(0, sep1);             // Ex: "08:00"
    String diasStr = linha.substring(sep1 + 1, sep2);       // Ex: "1,3,5"
    String audiosStr = linha.substring(sep2 + 1);           // Ex: "1x2,2x1"

    int h = horario.substring(0, 2).toInt(); // Extrai a hora
    int m = horario.substring(3, 5).toInt(); // Extrai os minutos
    sched.hour = h;
    sched.minute = m;

    while (diasStr.length() > 0) { // Converte a lista de dias
      int idx = diasStr.indexOf(',');
      int dia = (idx == -1 ? diasStr.toInt() : diasStr.substring(0, idx).toInt());
      if (sched.dayCount < 7) sched.days[sched.dayCount++] = dia;
      diasStr = (idx == -1 ? "" : diasStr.substring(idx + 1));
    }

    while (audiosStr.length() > 0) { // Converte a lista de audios
      int idx = audiosStr.indexOf(',');
      String itemStr = (idx == -1 ? audiosStr : audiosStr.substring(0, idx));
      int xPos = itemStr.indexOf('x');

      if (xPos != -1 && sched.audioCount < MAX_AUDIO_ITEMS) {
        int num = itemStr.substring(0, xPos).toInt();          // Numero do arquivo
        int reps = itemStr.substring(xPos + 1).toInt();        // Repeticoes
        sched.audioList[sched.audioCount++] = { num, reps };   // Adiciona o item a lista
      }

      audiosStr = (idx == -1 ? "" : audiosStr.substring(idx + 1));
    }

    schedules[scheduleCount++] = sched; // Armazena o agendamento no array
  }

  file.close(); // Fecha o arquivo apos leitura
  Serial.print("Configuracoes carregadas: ");
  Serial.println(scheduleCount); // Exibe no monitor serial o total de agendamentos lidos
}
