// Grupo 5: Tiziano Castro, Tomás Contreras y Tomas Pereyra

// Bibliotecas
#include <DHT.h>               // Sensor Temperatura
#include <Wire.h>              // Pantalla
#include <Adafruit_GFX.h>      // Pantalla
#include <Adafruit_SSD1306.h>  // Pantalla
#include <Preferences.h>       // Eeprom

typedef enum {
  RST,
  P1,
  P2,
  Bo1,
  Bo2,
  Espera,
  Espera2,
} estadoPrograma;
estadoPrograma estadoActual = RST;

// Defines OLED
#define ANCHO 128
#define ALTO 64
Adafruit_SSD1306 display(ANCHO, ALTO, &Wire, -1);

// Defines DHT
#define DHTPIN 23
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Defines botones y led
#define B1 35
#define B2 34
#define ESPERA_PULSO 30
#define PARAMETRO_1S 1000
#define PARAMETRO_5S 5000
#define PARAMETRO_SOLTAR 100
#define pin_LED 26

// Defines temperatura
float verificarTemperaturaNumerica;
float temperatura;
int umbral;

// Variables botones y tiempo
bool primerPulso;
unsigned long tiempoPulso;
unsigned long desfasaje;
unsigned long tiempoMillis;
unsigned long tiempoLiberacion;

// Eeprom
Preferences almacenamiento;

void setup() {  // se usa para definir pines de leds, el serial begin y se crea las tareas
  Serial.begin(115200);

  // LED Y BOTONES
  pinMode(pin_LED, OUTPUT);
  pinMode(B1, INPUT);
  pinMode(B2, INPUT);

  // DHT
  dht.begin();

  // OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // EEPROM
  almacenamiento.begin("C1TP4G5", false); // false permite leer y escribir, true solo leer
}

void loop() {
  switch (estadoActual) {
    case (RST):
      tiempoPulso = 0;
      tiempoLiberacion = 0;
      primerPulso = false;
      temperatura = 0;
      umbral = almacenamiento.getInt("umbral", 25); // Se elige el valor guardado del umbral si hay, y si no un valor defecto
      desfasaje = millis();
      estadoActual = P1;
      break;


    case (P1):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("VA (valor actual) : ");
      display.println(String(temperatura) + " *C");
      display.println("VU (valor umbral) : ");
      display.print(String(umbral) + " *C");
      display.display();
      if (digitalRead(B1) == LOW && primerPulso == false) {
        tiempoPulso = millis();
        primerPulso = true;
      }
      if (primerPulso == true) {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B1) == LOW) {
            primerPulso = false;
            tiempoMillis = 0;
            desfasaje = millis();
            estadoActual = Espera;
          }
        } else {
          if (digitalRead(B1) == HIGH) {
            primerPulso = false;
          }
        }
      }
      break;


    case (P2):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("VU (valor umbral) : ");
      display.print(String(umbral) + " *C");
      display.display();
      if (primerPulso == false) {
        if (digitalRead(B1) == LOW && digitalRead(B2) == HIGH) {
          tiempoPulso = millis();
          primerPulso = true;
        } else if (digitalRead(B1) == HIGH && digitalRead(B2) == LOW) {
          tiempoPulso = millis();
          primerPulso = true;
        }
      } else {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B1) == LOW && digitalRead(B2) == HIGH) {
            primerPulso = false;
            estadoActual = Bo1;
          } else if (digitalRead(B1) == HIGH && digitalRead(B2) == LOW) {
            primerPulso = false;
            estadoActual = Bo2;
            tiempoMillis = millis();
          }
        } else {
          if (digitalRead(B1) == HIGH && digitalRead(B2) == HIGH) {
            primerPulso = false;
          }
        }
      }
      break;


    case (Bo1):
      if (primerPulso == false) {
        if (digitalRead(B1) == HIGH) {
          tiempoPulso = millis();
          primerPulso = true;
        }
      } else {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B1) == HIGH) {
            primerPulso = false;
            estadoActual = P2;
            umbral = umbral + 1;
          }
        } else {
          if (digitalRead(B1) == LOW) {
            primerPulso = false;
          }
        }
      }
      break;


    case (Bo2):  
      if (primerPulso == false) {
        if (digitalRead(B2) == HIGH) {
          tiempoPulso = millis();
          primerPulso = true;
        } else if (digitalRead(B2) == LOW) {
          if (millis() - tiempoMillis >= PARAMETRO_1S) {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);
            display.println("esperando a que se");
            display.println("mantenga el boton 2");
            display.print("por 5 segundos");
            display.display();
          }
          if (millis() - tiempoMillis >= PARAMETRO_5S) {
            primerPulso = false;
            estadoActual = P1;
            almacenamiento.putInt("umbral", umbral); // guardamos el nuevo valor del umbral
          }
        }
      } else {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B2) == HIGH) {
            primerPulso = false;
            estadoActual = P2;
            umbral = umbral - 1;
          }
        } else {
          if (digitalRead(B2) == LOW) {
            primerPulso = false;
          }
        }
      }
      break;


    case (Espera):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("esperando a que se");
      display.println("mantenga el boton 1");
      display.print("por 5 segundos");
      display.display();
      tiempoMillis = millis() - desfasaje;
      if (digitalRead(B1) == LOW && tiempoMillis >= PARAMETRO_5S) {
        estadoActual = P2;
        tiempoLiberacion = 0;
      } else if (digitalRead(B1) == HIGH) {
        if (tiempoLiberacion == 0) {
          tiempoLiberacion = millis();
        }
        if (millis() - tiempoLiberacion >= PARAMETRO_SOLTAR) {
          tiempoLiberacion = 0;
          estadoActual = Espera2;
        }
      }
      break;


    case (Espera2):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("ya pasaron 5 segundos");
      display.print("suelte el boton 1");
      display.display();
      if (digitalRead(B1) == HIGH && primerPulso == false) {
        tiempoPulso = millis();
        primerPulso = true;
      }
      if (primerPulso == true) {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B1) == HIGH) {
            primerPulso = false;
            estadoActual = P1;
          }
        } else {
          if (digitalRead(B1) == LOW) {
            primerPulso = false;
          }
        }
      }
      break;
  }


  verificarTemperaturaNumerica = dht.readTemperature();
  if (!isnan(verificarTemperaturaNumerica)) {
    temperatura = verificarTemperaturaNumerica;
  }
  if (temperatura > umbral) {
    digitalWrite(pin_LED, HIGH);
  } else {
    digitalWrite(pin_LED, LOW);
  }
}
