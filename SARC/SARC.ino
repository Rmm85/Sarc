// s’inclouen les llibreries necessàries per al projecte
#include <SoftwareSerial.h> // Llibreria per a la comunicació sèrie amb el mòdul Bluetooth
#include <DHT.h> // Llibreria per al sensor DHT
#include <Wire.h> // Llibreria per habilitar les connexions I2C
#include <TimeLib.h> // Llibreria per gestionar l'hora

#define DHTTYPE DHT11
#define DHTPIN 12
#define sensor_humitat_pin A0
#define sensor_llum_pin A1

// Declaració de les variables necessàries per al programa
int llindar_temperatura = 30;
int llindar_temperatura_usuari = 30;
int llindar_humitat = 70;
int llindar_humitat_usuari = 70;
int llindar_humitat_terra = 55;
int llindar_humitat_terra_usuari = 55;
int llindar_lluminositat = 50;
int llindar_lluminositat_usuari = 50;
int rele = 3;
char inchar;

// Crea una instància del sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Defineix les posicions tx i rx del Bluetooth
int bluetoothTx = 7;
int bluetoothRx = 8;
SoftwareSerial blue(bluetoothTx, bluetoothRx);

unsigned long lastSerialTime = 0; // Temps de la última transmissió pel port sèrie
const unsigned long serialInterval = 900000; // Interval de 15 minuts (15 * 60 * 1000)

void setup() {
  Serial.begin(9600);
  blue.begin(9600);
  dht.begin();
  Wire.begin();

  // Inicialitza la comunicació amb l'hora i data actuals
  setTime(0, 0, 0, 1, 1, 2023); // Estableix l'hora inicial
  pinMode(rele, OUTPUT); // Configura el pin del relé com a sortida
}

void loop() {
  // Obté l'hora actual
  int hora = hour();
  int minut = minute();
  int segon = second();

  // Realitza la lectura d'humitat, temperatura, percentatge d'humitat en terra i llum
  int humitat = dht.readHumidity();
  int temperatura = dht.readTemperature();
  int humitat_terra = map(analogRead(sensor_humitat_pin), 0, 1023, 100, 0);
  int lluminositat = map(analogRead(sensor_llum_pin), 1023, 0, 0, 100);

  // Obté l'estat del relé
  int estat_rele = digitalRead(rele);

  // Verifica si ha passat el temps suficient per enviar el missatge pel port sèrie
  if (millis() - lastSerialTime >= serialInterval) {
    // Construeix el missatge a enviar pel port sèrie
    String TextPerSerial = "";
    TextPerSerial += "Hora: ";
    TextPerSerial += hora;
    TextPerSerial += ":";
    TextPerSerial += minut;
    TextPerSerial += ":";
    TextPerSerial += segon;
    TextPerSerial += " | Temperatura: ";
    TextPerSerial += temperatura;
    TextPerSerial += "°C | Humitat ambiental: ";
    TextPerSerial += humitat;
    TextPerSerial += "% | Humitat de terra: ";
    TextPerSerial += humitat_terra;
    TextPerSerial += "% | Lluminositat: ";
    TextPerSerial += lluminositat;
    TextPerSerial += "% | Llindars - Humitat ambiental: ";
    TextPerSerial += llindar_humitat;
    TextPerSerial += "%, Temperatura: ";
    TextPerSerial += llindar_temperatura;
    TextPerSerial += "°C, Humitat de terra: ";
    TextPerSerial += llindar_humitat_terra;
    TextPerSerial += "%, Lluminositat: ";
    TextPerSerial += llindar_lluminositat;
    TextPerSerial += "% | Estat del relé: ";
    TextPerSerial += (estat_rele == HIGH) ? "Actiu" : "Inactiu";

    // Envia el missatge pel port sèrie
    Serial.println(TextPerSerial);

    // Actualitza el temps de la darrera transmissió pel port sèrie
    lastSerialTime = millis();
  }

  String TextForBlue;
  TextForBlue = TextForBlue;
  TextForBlue.concat(humitat);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(temperatura);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(humitat_terra);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(lluminositat);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(llindar_humitat);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(llindar_temperatura);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(llindar_humitat_terra);
  TextForBlue = TextForBlue + ";";
  TextForBlue.concat(llindar_lluminositat);
  TextForBlue = TextForBlue + ";";
  blue.print(TextForBlue);
  TextForBlue = " ";
  delay(2000);

  // Modifica els llindars per iniciar el reg automàtic
  if (blue.available()) {
    inchar = blue.read();

    switch (inchar) {
      case 'A':
        llindar_temperatura_usuari = blue.read();
        llindar_temperatura = llindar_temperatura_usuari;
        break;

      case 'B':
        llindar_humitat_usuari = blue.read();
        llindar_humitat = llindar_humitat_usuari;
        break;

      case 'C':
        llindar_humitat_terra_usuari = blue.read();
        llindar_humitat_terra = llindar_humitat_terra_usuari;
        break;

      case 'D':
        llindar_lluminositat_usuari = blue.read();
        llindar_lluminositat = llindar_lluminositat_usuari;
        break;

      case 'E':
        llindar_humitat = 70;
        llindar_temperatura = 30;
        llindar_humitat_terra = 50;
        llindar_lluminositat = 55;
        break;

      case 'F':
        digitalWrite(rele, HIGH); // Activa el relé
        llindar_humitat = llindar_humitat_usuari;
        llindar_temperatura = llindar_temperatura_usuari;
        llindar_humitat_terra = llindar_humitat_terra_usuari;
        llindar_lluminositat = llindar_lluminositat_usuari;
        break;

      case 'G':
        digitalWrite(rele, LOW); // Desactiva el relé
        llindar_humitat = 0;
        llindar_temperatura = 0;
        llindar_humitat_terra = 0;
        llindar_lluminositat = 0;
        break;
    }
  }

  // Comprova si es superen els llindars i activa o desactiva el relé
  if (temperatura <= llindar_temperatura && humitat <= llindar_humitat && humitat_terra <= llindar_humitat_terra && lluminositat <= llindar_lluminositat) {
    digitalWrite(rele, HIGH); // Activa el relé
  } else {
    digitalWrite(rele, LOW); // Desactiva el relé
  }
  delay(500);
}


