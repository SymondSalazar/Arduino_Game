#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración del LCD (20 columnas, 4 filas)
// Si 0x27 no funciona, prueba con 0x3F
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Configuración de botones
const int botonIncremento = 7;
const int botonDecremento = 8;
const int botonDisparar = 9;
bool juegoEmpezado = false;
unsigned long tiempoInicio = 0;
unsigned long intervaloTime = 1000; // Intervalo de actualización del tiempo en milisegundos
unsigned long intervaloPersonaje = 250;
unsigned long ultimaActualizacion = 0;
unsigned long ultimaActualizacionPersonaje = 0;
unsigned long ultimoIncremento = 0;
const unsigned long debounceDelay = 350;
bool blocMovimiento = false;
bool balaDisparada = false;
int posicionPersonaje = 1; // Posición del personaje en la pantalla
int posicionBalaY;
int posicionBalaX = 1;

void setup() {
  Serial.begin(9600); // Para depuración

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Verificar conexión LCD
  lcd.print("Iniciando...");
  delay(1000);

  pinMode(botonIncremento, INPUT);
  pinMode(botonDecremento, INPUT);
  pinMode(botonDisparar, INPUT);

  mostrarBienvenida();
}

void loop() {
  if (juegoEmpezado) {
    juego();
  } else {
    mostrarBienvenida();
  }
}

void mostrarBienvenida() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("to the Game!");
  lcd.setCursor(0, 2);
  lcd.print("Press a button");
  lcd.setCursor(0, 3);
  lcd.print("to start =)");

  while (digitalRead(botonIncremento) == LOW &&
         digitalRead(botonDecremento) == LOW &&
         digitalRead(botonDisparar) == LOW) {
    delay(100); // Esperar hasta que se presione un botón
  }

  lcd.clear();
  juegoEmpezado = true;
  tiempoInicio = millis();

  juego();
}

void juego() {
  unsigned long ahora = millis();

  if (ahora - ultimaActualizacion >= intervaloTime) {
    mostrarTiempo();
    ultimaActualizacion = ahora;
  }

  if (ahora - ultimaActualizacionPersonaje >= intervaloPersonaje) {
    dibujarEscena();
    ultimaActualizacionPersonaje = ahora;
    if (balaDisparada) balaControl();
  }

  personajeControl();
}

void mostrarTiempo() {
  unsigned long ahora = millis();
  unsigned long transcurrido = (ahora - tiempoInicio) / 1000;
  int minutos = transcurrido / 60;
  int segundos = transcurrido % 60;

  lcd.setCursor(0, 0);
  lcd.print("                    "); // Limpiar la primera línea
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) {
    lcd.print("0"); // Añadir cero delante si es necesario
  }
  lcd.print(segundos);
}

void personajeControl() {
  unsigned long ahora = millis();
  if (digitalRead(botonIncremento) == HIGH && (ahora - ultimoIncremento > debounceDelay)) {
    ultimoIncremento = ahora;
    posicionPersonaje = (posicionPersonaje + 1) % 3;
    
  }

  if (digitalRead(botonDecremento) == HIGH && (ahora - ultimoIncremento > debounceDelay)) {
    ultimoIncremento = ahora;
    posicionPersonaje = (posicionPersonaje - 1 + 3) % 3;
  }

  if (digitalRead(botonDisparar) == HIGH && !balaDisparada) {
    posicionBalaY = posicionPersonaje;
    balaDisparada = true;
  }
}

void balaControl() {
  if (posicionBalaX < 19) {
    posicionBalaX++;
  } else {
    balaDisparada = false;
    posicionBalaX = 1;
  }
}

void enemigoControl() {
  // En desarrollo
}

void dibujarEscena() {
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");

  for (int i = 0; i < 3; i++) {
    lcd.setCursor(0, i + 1);
    if (i == posicionPersonaje) {
      lcd.print(">");
    } else {
      lcd.print(" ");
    }

    if (balaDisparada && i == posicionBalaY) {
      lcd.setCursor(posicionBalaX, i + 1);
      lcd.print("-");
    }
  }

  blocMovimiento = false;
}