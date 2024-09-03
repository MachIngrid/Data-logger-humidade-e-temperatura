#include <Wire.h> // Inclusão da biblioteca Wire para comunicação I2C
#include "RTClib.h" // Inclusão da biblioteca para o RTC DS3231
#include "DHT.h"
#define DHTPIN 2     // Pino digital sensor DHT

// Escolha o tipo de sensor!
# define DHTTYPE DHT11     // DHT 11
//# define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321
//# define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

#include <Wire.h>
RTC_DS3231 rtc; // Objeto do tipo RTC_DS3231

// Declaração dos dias da semana
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};


// Tamanho da fila circular
const int bufferSize = 10;
String buffer[bufferSize];
int bufferIndex = 0;

bool monitoring = false; // Variável para controlar o monitoramento

// Função para iniciar o sistema de monitoramento
void startMonitoring() {
  monitoring = true;
  Serial.println("Monitoramento iniciado.");
}

// Função para pausar o sistema de monitoramento
void pauseMonitoring() {
  monitoring = false;
  Serial.println("Monitoramento pausado.");
}

// Função para adicionar valores na fila circular
void addToBuffer(String data) {
  buffer[bufferIndex] = data;
  bufferIndex = (bufferIndex + 1) % bufferSize;
}

// Função para exibir os valores armazenados na fila circular
void displayBuffer() {
  Serial.println("Valores armazenados na fila circular:");
  for (int i = 0; i < bufferSize; i++) {
    if (buffer[i] != "") {  // Exibe apenas entradas válidas
      Serial.println(buffer[i]);
    }
  }
}

void setup() {
  Serial.begin(9600); // Inicializa a comunicação serial
  Wire.begin(); // Inicializa a comunicação I2C
  Serial.println(F("DHT teste!"));

  dht.begin();


  if (!rtc.begin()) {
    Serial.println("RTC não encontrado!");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("Perda de energia detectada, configurando o RTC...");
    //rtc.adjust(DateTime(F(_DATE), F(TIME_))); // Ajusta o RTC para a data e hora da compilação
    //rtc.adjust(DateTime(2024, 9, 03, 15, 57, 45)); // Exemplo de ajuste manual
  }

 
  String intro = """Datalogger ok. Pressione :'1' - para iniciar '2' - para pausar '3' - para exibir valores armazenados.""";

  Serial.println(intro);
}

void loop() {
  float h = dht.readHumidity();
      // Temperature em Celsius (default)
  float t = dht.readTemperature();
  
  // Verifique se alguma leitura falhou e tenta novamente.
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Falha de leitura do sensor DHT!"));
    return;
  }
   float hic = dht.computeHeatIndex(t, h, false);
   if (Serial.available()) {
    char command = Serial.read();
    if (command == '1') {
      startMonitoring();
    } else if (command == '2') {
      pauseMonitoring();
    } else if (command == '3') {
      displayBuffer();
    }
  }

  if (monitoring) {
  
    float hic = dht.computeHeatIndex(t, h, false);
    DateTime now = rtc.now(); // Obtém a data e hora atuais do RTC

    // Formata a string com a data, hora e valor do LDR
    String data = "Data: " + String(now.day(), DEC) + "/" + String(now.month(), DEC) + "/" + String(now.year(), DEC) +
                  " / Dia: " + String(daysOfTheWeek[now.dayOfTheWeek()]) +
                  " / Horas: " + String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC) +
                  " - Temperatura: " + String(t) + "°C" +
                  " - Humidade: " + String(h) + "%";

    addToBuffer(data); // Adiciona a leitura à fila circular

    Serial.println(data); // Exibe a leitura no monitor serial

    delay(10000); // Intervalo de 1 segundo entre as leituras
  }
}
