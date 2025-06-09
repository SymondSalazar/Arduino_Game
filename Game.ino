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
bool enemigosGen = false;
unsigned long tiempoInicio = 0;
unsigned long intervaloTime = 1000; // Intervalo de actualización del tiempo en milisegundos
unsigned long intervaloPersonaje = 250;
unsigned long ultimaActualizacion = 0;
unsigned long ultimaActualizacionPersonaje = 0;
unsigned long ultimoIncremento = 0;
const unsigned long debounceDelay = 300;
bool balaDisparada = false;
int posicionPersonaje = 1; // Posición del personaje en la pantalla
int posicionBalaY;
int posicionBalaX = 1;
int minutos;
int segundos;
int contadorKills;
int posiciones[] = {19,19,19,19,19,19};
int posicionesY[]= {0,0,1,1,2,2};
int espacios = 4;


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

  personajeControl();

  if (ahora - ultimaActualizacion >= intervaloTime) {
    mostrarTiempo();
    ultimaActualizacion = ahora;
  }

  if (ahora - ultimaActualizacionPersonaje >= intervaloPersonaje) {
    if (balaDisparada) balaControl();
    enemigoControl();
    comprobarColisiones();
    dibujarEscena();
    ultimaActualizacionPersonaje = ahora;
  }
}

void mostrarTiempo() {
  unsigned long ahora = millis();
  unsigned long transcurrido = (ahora - tiempoInicio) / 1000;
  minutos = transcurrido / 60;
  segundos = transcurrido % 60;

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
    balaDisparada = true;
    posicionBalaY = posicionPersonaje;
    posicionBalaX = 1;
    
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
  if(!enemigosGen){
    posiciones[0] = 19;
    posiciones[1] = 27;
    posiciones[2] = 23 + espacios;
    posiciones[3] = 19 + espacios * random(1, 3);
    posiciones[4] = 19 + espacios * 2 ;
    posiciones[5] = 19 + espacios * 2 * random(0, 3);
    enemigosGen = true;
  }
  else{
    for(int i = 0; i < 6; i++){
      if (posiciones[i] > 0) {
        posiciones[i]--;
      } else {
        posiciones[i] = 19 + espacios * random(0, 3); // Reubicar enemigo fuera de la pantalla
      }
    }
  }
}

void comprobarColisiones() {
  for (int i = 0; i < 6; i++) {
    // Verifica si la bala está en el rango del enemigo (±1)
    if (balaDisparada && 
        abs(posiciones[i] - posicionBalaX) <= 1 && 
        posicionesY[i] == posicionBalaY) 
    {
      contadorKills++;
      posiciones[i] = 19 + espacios * random(0, 3);
      balaDisparada = false;
    }

    // Colisión con personaje (incluye rango)
    if (posiciones[i] <= 0 && posicionesY[i] == posicionPersonaje) {
      juegoEmpezado = false;
      gameOver();
    }
  }
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

    for(int j = 0; j<6;j++){
      if(posiciones[j]>= 0 && posiciones[j]<20){
        lcd.setCursor(posiciones[j], posicionesY[j] + 1);
        lcd.print("X");
      }
    }
  }
}

void gameOver(){

  posicionPersonaje = 1;
  balaDisparada = false;
  enemigosGen = false;
  tiempoInicio = millis();
  for (int i = 0; i < 6; i++) {
    posiciones[i] = 19;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-----Game  Over-----");
  
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) {
    lcd.print("0"); // Añadir cero delante si es necesario
  }
  lcd.print(segundos);
  
  
  lcd.setCursor(0, 2);
  lcd.print("Kills: ");
  lcd.print(contadorKills);
  
  lcd.setCursor(0, 3);
  long score = (minutos*60*10) + (segundos*10) + contadorKills*100;
  lcd.print("Score: ");
  lcd.print(score);
  contadorKills = 0;
  while (digitalRead(botonIncremento) == LOW &&
         digitalRead(botonDecremento) == LOW &&
         digitalRead(botonDisparar) == LOW) {
    delay(100); // Esperar hasta que se presione un botón
  }

  delay(500);
}