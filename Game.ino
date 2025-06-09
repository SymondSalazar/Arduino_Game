#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =============================================
// CONFIGURACIÓN HARDWARE
// =============================================
LiquidCrystal_I2C lcd(0x27, 20, 4);  // LCD 20x4

// Pines de botones
const int botonIncremento = 7;
const int botonDecremento = 8;
const int botonDisparar   = 9;

// =============================================
// VARIABLES DEL JUEGO
// =============================================
// Estados y controles
bool juegoEmpezado = false;
bool enemigosGen   = false;
bool balaDisparada = false;

// Tiempos y intervalos
unsigned long tiempoInicio              = 0;
unsigned long intervaloTime             = 1000;   // Actualización tiempo (ms)
unsigned long intervaloPersonaje        = 250;    // Actualización personaje (ms)
unsigned long ultimaActualizacion       = 0;
unsigned long ultimaActualizacionPersonaje = 0;
unsigned long ultimoIncremento          = 0;
const unsigned long debounceDelay       = 350;    // Debounce botones

// Posiciones de elementos
int posicionPersonaje = 1;  // 0=arriba, 1=centro, 2=abajo
int posicionBalaY;
int posicionBalaX = 1;

// Estadísticas
int minutos;
int segundos;
int contadorKills;

// Sistema de enemigos
int posiciones[]    = {19, 19, 19, 19, 19, 19};  // Posiciones X
int posicionesY[]   = {0, 0, 1, 1, 2, 2};        // Posiciones Y (filas)
int espacios        = 4;                           // Espaciado entre enemigos

// =============================================
// CONFIGURACIÓN INICIAL
// =============================================
void setup() {
  Serial.begin(9600);
  
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Iniciando...");
  delay(1000);

  // Configurar pines de botones
  pinMode(botonIncremento, INPUT);
  pinMode(botonDecremento, INPUT);
  pinMode(botonDisparar, INPUT);

  mostrarBienvenida();
}

// =============================================
// BUCLE PRINCIPAL
// =============================================
void loop() {
  if (juegoEmpezado) {
    juego();
  } else {
    mostrarBienvenida();
  }
}

// =============================================
// PANTALLAS DEL JUEGO
// =============================================
void mostrarBienvenida() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Welcome");
  lcd.setCursor(0, 1); lcd.print("to the Game!");
  lcd.setCursor(0, 2); lcd.print("Press a button");
  lcd.setCursor(0, 3); lcd.print("to start =)");

  // Esperar acción del jugador
  while (digitalRead(botonIncremento) == LOW &&
         digitalRead(botonDecremento) == LOW &&
         digitalRead(botonDisparar) == LOW) {
    delay(100);
  }

  // Iniciar juego
  lcd.clear();
  juegoEmpezado = true;
  tiempoInicio = millis();
}

void gameOver() {
  // Reiniciar variables del juego
  posicionPersonaje = 1;
  balaDisparada = false;
  enemigosGen = false;
  tiempoInicio = millis();
  
  for (int i = 0; i < 6; i++) {
    posiciones[i] = 19;
  }

  // Mostrar pantalla de fin
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("-----Game  Over-----");
  
  // Mostrar tiempo sobrevivido
  lcd.setCursor(0, 1); 
  lcd.print("Time: ");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) lcd.print("0");
  lcd.print(segundos);
  
  // Mostrar enemigos eliminados
  lcd.setCursor(0, 2); 
  lcd.print("Kills: ");
  lcd.print(contadorKills);
  
  // Calcular y mostrar puntuación
  lcd.setCursor(0, 3);
  long score = (minutos * 60 * 10) + (segundos * 10) + contadorKills * 100;
  lcd.print("Score: ");
  lcd.print(score);
  
  contadorKills = 0;

  // Esperar reinicio
  while (digitalRead(botonIncremento) == LOW &&
         digitalRead(botonDecremento) == LOW &&
         digitalRead(botonDisparar) == LOW) {
    delay(100);
  }
  delay(500);
}

// =============================================
// LÓGICA PRINCIPAL DEL JUEGO
// =============================================
void juego() {
  unsigned long ahora = millis();

  // 1. Control del personaje
  personajeControl();

  // 2. Actualizar tiempo periódicamente
  if (ahora - ultimaActualizacion >= intervaloTime) {
    mostrarTiempo();
    ultimaActualizacion = ahora;
  }

  // 3. Actualizar elementos del juego
  if (ahora - ultimaActualizacionPersonaje >= intervaloPersonaje) {
    // Secuencia de actualización del juego
    if (balaDisparada) balaControl();
    enemigoControl();
    comprobarColisiones();
    dibujarEscena();
    
    ultimaActualizacionPersonaje = ahora;
  }
}

// =============================================
// SISTEMA DE TIEMPO
// =============================================
void mostrarTiempo() {
  unsigned long ahora = millis();
  unsigned long transcurrido = (ahora - tiempoInicio) / 1000;
  
  minutos = transcurrido / 60;
  segundos = transcurrido % 60;

  // Mostrar tiempo formateado
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(minutos);
  lcd.print(":");
  if (segundos < 10) lcd.print("0");
  lcd.print(segundos);
}

// =============================================
// CONTROLES DE PERSONAJE
// =============================================
void personajeControl() {
  unsigned long ahora = millis();
  
  // Movimiento ascendente
  if (digitalRead(botonIncremento) == HIGH && 
      (ahora - ultimoIncremento > debounceDelay)) {
    ultimoIncremento = ahora;
    posicionPersonaje = (posicionPersonaje + 1) % 3;
  }

  // Movimiento descendente
  if (digitalRead(botonDecremento) == HIGH && 
      (ahora - ultimoIncremento > debounceDelay)) {
    ultimoIncremento = ahora;
    posicionPersonaje = (posicionPersonaje - 1 + 3) % 3;
  }

  // Disparar
  if (digitalRead(botonDisparar) == HIGH && !balaDisparada) {
    balaDisparada = true;
    posicionBalaY = posicionPersonaje;
    posicionBalaX = 1;
  }
}

// =============================================
// SISTEMA DE BALAS
// =============================================
void balaControl() {
  if (posicionBalaX < 19) {
    posicionBalaX++;
  } else {
    // Resetear bala al salir de pantalla
    balaDisparada = false;
    posicionBalaX = 1;
  }
}

// =============================================
// SISTEMA DE ENEMIGOS
// =============================================
void enemigoControl() {
  if (!enemigosGen) {
    // Configuración inicial de enemigos
    posiciones[0] = 19;
    posiciones[1] = 27;
    posiciones[2] = 23 + espacios;
    posiciones[3] = 19 + espacios * random(1, 3);
    posiciones[4] = 19 + espacios * 2;
    posiciones[5] = 19 + espacios * 2 * random(0, 3);
    enemigosGen = true;
  }
  else {
    // Movimiento de enemigos
    for (int i = 0; i < 6; i++) {
      if (posiciones[i] > 0) {
        posiciones[i]--;
      } else {
        // Reubicar enemigo fuera de pantalla
        posiciones[i] = 19 + espacios * random(0, 3);
      }
    }
  }
}

// =============================================
// DETECCIÓN DE COLISIONES
// =============================================
void comprobarColisiones() {
  for (int i = 0; i < 6; i++) {
    // Colisión bala-enemigo (con margen de error)
    if (balaDisparada && 
        abs(posiciones[i] - posicionBalaX) <= 1 && 
        posicionesY[i] == posicionBalaY) 
    {
      contadorKills++;
      posiciones[i] = 19 + espacios * random(0, 3);
      balaDisparada = false;
    }

    // Colisión personaje-enemigo
    if (posiciones[i] <= 0 && posicionesY[i] == posicionPersonaje) {
      juegoEmpezado = false;
      gameOver();
    }
  }
}

// =============================================
// SISTEMA DE RENDERIZADO
// =============================================
void dibujarEscena() {
  // Limpiar áreas de juego
  lcd.setCursor(0, 1); lcd.print("                    ");
  lcd.setCursor(0, 2); lcd.print("                    ");
  lcd.setCursor(0, 3); lcd.print("                    ");

  // Dibujar elementos en cada fila
  for (int i = 0; i < 3; i++) {
    // Dibujar personaje
    lcd.setCursor(0, i + 1);
    lcd.print((i == posicionPersonaje) ? ">" : " ");
    
    // Dibujar bala
    if (balaDisparada && i == posicionBalaY) {
      lcd.setCursor(posicionBalaX, i + 1);
      lcd.print("-");
    }
    
    // Dibujar enemigos
    for (int j = 0; j < 6; j++) {
      if (posiciones[j] >= 0 && posiciones[j] < 20) {
        lcd.setCursor(posiciones[j], posicionesY[j] + 1);
        lcd.print("X");
      }
    }
  }
}