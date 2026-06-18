// Grupo 5: Tiziano Castro, Tomás Contreras y Tomas Pereyra

// Bibiliotecas
#include <DHT.h>               //Sensor Temperatura
#include <Wire.h>              // Pantalla
#include <Adafruit_GFX.h>      // Pantalla
#include <Adafruit_SSD1306.h>  // Pantalla

typedef enum {
  RST,
  P1,
  P2,
  Bo1,
  Bo2,
  Espera1,
  Espera2,
  Espera3,
  Espera4,
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
#define PARAMETRO_5S 5000
#define PARAMETRO_SOLTAR 100
#define pin_LED 26

// Defines temperatura
float verificarTemperaturaNumerica;
float temperatura;
int umbral;

// Variables botones y tiempo
unsigned long tiempoPulso;
bool primerPulso;
unsigned long desfasaje;
int tiempoMillis;
unsigned long tiempoSuelto;
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
}

void loop() {
  switch (estadoActual) {
    case (RST):
      tiempoPulso = 0;
      tiempoSuelto = 0;
      primerPulso = false;
      temperatura = 0;
      umbral = 25;
      desfasaje = millis();
      estadoActual = P1;
      break;


    case (P1):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("VA (valor actual) : " + String(temperatura) + " *C");
      display.print("VU (valor umbral) : " + String(umbral) + " *C");
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
            estadoActual = Espera1;
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
      display.print("valor umbral: " + String(umbral) + " *C");
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
        }
      } else {
        if (digitalRead(B2) == LOW && millis() - tiempoPulso >= PARAMETRO_MILLIS) {
          primerPulso = false;
          estadoActual = P1;
        }
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


    case (Espera1):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("esperando a que se mantenga");
      display.print("el botón 1 por 5 segundos");
      display.display();
      tiempoMillis = millis() - desfasaje;
      if (digitalRead(B1) == LOW && tiempoMillis >= PARAMETRO_MILLIS) {
        estadoActual = P2;
        tiempoSuelto = 0;
      }
      else if (digitalRead(B1) == HIGH) {
        if (inicioLiberacion == 0) {
          inicioLiberacion = millis();
        }
        if (millis() - inicioLiberacion >= PARAMETRO_SOLTAR) {
          inicioLiberacion = 0;
          estadoActual = P1;
        }
      }
      break;


    case (Espera6):
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("esperando...");
      display.display();
      if (digitalRead(B1) == HIGH && digitalRead(B2) == HIGH && primerPulso == false) {
        tiempoPulso = millis();
        primerPulso = true;
      }
      if (primerPulso == true) {
        if (millis() - tiempoPulso >= ESPERA_PULSO) {
          if (digitalRead(B1) == HIGH && digitalRead(B2) == HIGH) {
            primerPulso = false;
            estadoActual = P1;
          }
        } else {
          if (digitalRead(B1) == LOW || digitalRead(B2) == LOW) {
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
