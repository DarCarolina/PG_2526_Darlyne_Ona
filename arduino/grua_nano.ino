/**
 * @file grua_nano.ino
 * @author Antigravity AI
 * @brief Firmware para el control de la Grúa Torre utilizando Arduino Nano.
 * 
 * LÓGICA DE CONTROL:
 * Este código gestiona tres movimientos principales:
 * 1. Carro (Motor DC A): Adelante/Atrás (Driver TB6612FNG).
 * 2. Elevación (Motor DC B): Subir/Bajar (Driver TB6612FNG).
 * 3. Giro (Motor a Pasos): Izquierda/Derecha (Driver DRV8825).
 * 
 * El sistema permite un control dual:
 * - Manual: Lectura de Joysticks analógicos en A0, A1, A2.
 * - Remoto: Comandos seriales recibidos desde el ESP32 (TX ESP32 -> RX Nano).
 */

#include <AccelStepper.h>

// --- Definición de Pines ---

// Pines para el Driver TB6612FNG (Motores DC N20)
const int AIN1 = 2;   // Control de dirección Motor A (Carro)
const int AIN2 = 4;
const int PWMA = 3;   // Control de velocidad PWM Motor A
const int BIN1 = 7;   // Control de dirección Motor B (Elevación)
const int BIN2 = 8;
const int PWMB = 5;   // Control de velocidad PWM Motor B
const int STBY = 6;   // Pin de Standby del driver (debe estar en HIGH para operar)

// Pines para el Driver DRV8825 (Motor a Pasos Nema 17 - Giro)
const int STEP_PIN = 9;   // Pin de pulsos para el motor a pasos
const int DIR_PIN = 10;   // Pin de dirección para el motor a pasos

// Joysticks (Entradas Analógicas)
const int JOY_X = A0; // Joystick para el Carro
const int JOY_Y = A1; // Joystick para la Elevación
const int JOY_G = A2; // Joystick para el Giro

// --- Configuración de Objetos ---

// Inicialización del motor a pasos para el Giro utilizando la librería AccelStepper
AccelStepper stepperGiro(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// --- Variables Globales ---

char lastSerialCmd = 'S';       // Almacena el último comando recibido vía Serial ('S' por defecto es STOP)
unsigned long lastCmdTime = 0;   // Registra el tiempo del último comando recibido para el timeout
const unsigned long CMD_TIMEOUT = 1000; // Tiempo de seguridad (1 segundo). Si no llega nada, se detiene.

void setup() {
  // Configuración de pines de salida para el driver TB6612FNG
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  
  // Activamos el driver sacándolo de modo Standby
  digitalWrite(STBY, HIGH); 

  // Iniciamos comunicación Serial a 9600 baudios para recibir datos del ESP32
  Serial.begin(9600);
  
  // Configuración inicial del motor a pasos (velocidad y aceleración máxima)
  stepperGiro.setMaxSpeed(1000);
  stepperGiro.setAcceleration(500);
}

void loop() {
  // 1. GESTIÓN DE COMANDOS SERIALES (Desde ESP32)
  if (Serial.available() > 0) {
    lastSerialCmd = Serial.read(); // Leemos el carácter enviado
    lastCmdTime = millis();        // Actualizamos el tiempo de recepción
  }

  // Lógica de Seguridad (Timeout): 
  // Si ha pasado más de 1 segundo desde el último comando remoto, forzamos STOP ('S')
  if (millis() - lastCmdTime > CMD_TIMEOUT) {
    lastSerialCmd = 'S';
  }

  // 2. LECTURA DE LOS JOYSTICKS (Control Manual)
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  int joyG = analogRead(JOY_G);

  // 3. PROCESAMIENTO DE MOVIMIENTOS
  // Llamamos a las funciones de control pasando tanto el valor del joystick como el comando serial
  controlCarro(joyX, lastSerialCmd);
  controlElevacion(joyY, lastSerialCmd);
  controlGiro(joyG, lastSerialCmd);

  // Ejecutamos el movimiento del motor a pasos (necesario para que AccelStepper funcione)
  stepperGiro.runSpeed();
}

/**
 * @brief Control del Motor A (Carro). Prioriza el Joystick si se sale de la zona muerta.
 * @param joy Valor analógico (0-1023). Zona muerta central ~512.
 * @param cmd Comando serial ('F' = Forward, 'B' = Backward, 'S' = Stop).
 */
void controlCarro(int joy, char cmd) {
  int speed = 0;
  
  // --- LÓGICA MANUAL (JOYSTICK) ---
  if (joy > 570) { // Mover adelante
    speed = map(joy, 570, 1023, 0, 255);
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (joy < 450) { // Mover atrás
    speed = map(joy, 450, 0, 0, 255);
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } 
  // --- LÓGICA REMOTA (WEB via ESP32) ---
  else if (cmd == 'F') { // Comando Adelante
    speed = 200;
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (cmd == 'B') { // Comando Atrás
    speed = 200;
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } 
  // --- PARADA ---
  else {
    speed = 0;
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  }
  
  // Aplicamos la velocidad mediante PWM
  analogWrite(PWMA, speed);
}

/**
 * @brief Control del Motor B (Elevación).
 * @param joy Valor analógico del joystick.
 * @param cmd Comando serial ('U' = Up, 'D' = Down).
 */
void controlElevacion(int joy, char cmd) {
  int speed = 0;
  
  // --- LÓGICA MANUAL ---
  if (joy > 570) { // Subir
    speed = map(joy, 570, 1023, 0, 255);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (joy < 450) { // Bajar
    speed = map(joy, 450, 0, 0, 255);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } 
  // --- LÓGICA REMOTA ---
  else if (cmd == 'U') { // Comando Subir
    speed = 200;
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (cmd == 'D') { // Comando Bajar
    speed = 200;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } 
  // --- PARADA ---
  else {
    speed = 0;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
  
  // Aplicamos la velocidad mediante PWM
  analogWrite(PWMB, speed);
}

/**
 * @brief Control del Giro de la base (Motor a Pasos).
 * @param joy Valor analógico del joystick.
 * @param cmd Comando serial ('L' = Left, 'R' = Right).
 */
void controlGiro(int joy, char cmd) {
  // El motor a pasos requiere que definamos la velocidad en pasos por segundo
  
  // --- LÓGICA MANUAL ---
  if (joy > 570) {
    stepperGiro.setSpeed(400); // Giro derecha manual
  } else if (joy < 450) {
    stepperGiro.setSpeed(-400); // Giro izquierda manual
  } 
  // --- LÓGICA REMOTA ---
  else if (cmd == 'L') {
    stepperGiro.setSpeed(-600); // Giro izquierda remoto (más rápido)
  } else if (cmd == 'R') {
    stepperGiro.setSpeed(600);  // Giro derecha remoto
  } 
  // --- PARADA ---
  else {
    stepperGiro.setSpeed(0);
  }
}
