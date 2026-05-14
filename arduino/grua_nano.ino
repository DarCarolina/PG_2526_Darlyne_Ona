/**
 * @file grua_nano.ino
 * @author Antigravity AI
 * @brief Firmware para el control de la Grúa Torre utilizando Arduino Nano.
 * 
 * Este código gestiona dos motores DC (Carro y Elevación) mediante el driver TB6612FNG
 * y un motor a pasos (Giro) mediante el driver DRV8825. Permite control dual:
 * 1. Manual: A través de Joysticks analógicos.
 * 2. Remoto: Comandos recibidos vía UART desde el ESP32.
 */

#include <AccelStepper.h>

// --- Definición de Pines ---

// Driver TB6612FNG (Motores DC N20)
const int AIN1 = 2;   // Motor A (Carro)
const int AIN2 = 4;
const int PWMA = 3;
const int BIN1 = 7;   // Motor B (Elevación)
const int BIN2 = 8;
const int PWMB = 5;
const int STBY = 6;   // Standby (opcional, conectado a 5V o pin digital)

// Driver DRV8825 (Motor a Pasos Nema 17 - Giro)
const int STEP_PIN = 9;
const int DIR_PIN = 10;

// Joysticks (Entradas Analógicas)
const int JOY_X = A0; // Carro
const int JOY_Y = A1; // Elevación
const int JOY_G = A2; // Giro

// --- Configuración de Objetos ---

// Motor a pasos Giro (DRV8825)
AccelStepper stepperGiro(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// --- Variables Globales ---

char lastSerialCmd = 'S'; // Último comando recibido vía Serial
unsigned long lastCmdTime = 0;
const unsigned long CMD_TIMEOUT = 1000; // Timeout de seguridad (1 segundo)

void setup() {
  // Configuración de pines de salida para TB6612FNG
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  
  digitalWrite(STBY, HIGH); // Habilitar driver TB6612FNG

  // Configuración de Serial
  Serial.begin(9600);
  
  // Configuración del Stepper
  stepperGiro.setMaxSpeed(1000);
  stepperGiro.setAcceleration(500);
}

void loop() {
  // 1. Lectura de Comandos Serial (ESP32)
  if (Serial.available() > 0) {
    lastSerialCmd = Serial.read();
    lastCmdTime = millis();
  }

  // Timeout de seguridad: Si no llega comando en 1s, parar (solo si el último fue remoto)
  if (millis() - lastCmdTime > CMD_TIMEOUT) {
    lastSerialCmd = 'S';
  }

  // 2. Lectura de Joysticks
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);
  int joyG = analogRead(JOY_G);

  // 3. Lógica de Control de Motores (Mix de Serial y Joystick)
  controlCarro(joyX, lastSerialCmd);
  controlElevacion(joyY, lastSerialCmd);
  controlGiro(joyG, lastSerialCmd);

  // Ejecutar paso del stepper si es necesario
  stepperGiro.runSpeed();
}

/**
 * @brief Controla el movimiento del Carro (Motor A).
 * @param joy Valor analógico del joystick (0-1023).
 * @param cmd Comando serial ('F' o 'B').
 */
void controlCarro(int joy, char cmd) {
  int speed = 0;
  
  // Lógica de Joystick (Umbral de zona muerta: 450-570)
  if (joy > 570) {
    speed = map(joy, 570, 1023, 0, 255);
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (joy < 450) {
    speed = map(joy, 450, 0, 0, 255);
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } 
  // Lógica de Serial (Sobrescribe si hay comando activo)
  else if (cmd == 'F') {
    speed = 200;
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else if (cmd == 'B') {
    speed = 200;
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  } else {
    speed = 0;
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  }
  
  analogWrite(PWMA, speed);
}

/**
 * @brief Controla el movimiento de Elevación (Motor B).
 * @param joy Valor analógico del joystick (0-1023).
 * @param cmd Comando serial ('U' o 'D').
 */
void controlElevacion(int joy, char cmd) {
  int speed = 0;
  
  if (joy > 570) {
    speed = map(joy, 570, 1023, 0, 255);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (joy < 450) {
    speed = map(joy, 450, 0, 0, 255);
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else if (cmd == 'U') {
    speed = 200;
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else if (cmd == 'D') {
    speed = 200;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  } else {
    speed = 0;
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  }
  
  analogWrite(PWMB, speed);
}

/**
 * @brief Controla el giro de la grúa (Motor a Pasos).
 * @param joy Valor analógico del joystick (0-1023).
 * @param cmd Comando serial ('L' o 'R').
 */
void controlGiro(int joy, char cmd) {
  if (joy > 570) {
    stepperGiro.setSpeed(400);
  } else if (joy < 450) {
    stepperGiro.setSpeed(-400);
  } else if (cmd == 'L') {
    stepperGiro.setSpeed(-600);
  } else if (cmd == 'R') {
    stepperGiro.setSpeed(600);
  } else {
    stepperGiro.setSpeed(0);
  }
}
