
# Proyecto Grúa Torre: Requerimientos Técnicos para Generación de Código (v2)
## Contexto del Proyecto
Este documento está optimizado para su procesamiento por agentes de IA en **Antigravity** o **Codex**. El objetivo es generar el firmware para una grúa torre con control dual (Manual vía Joysticks y Remoto vía Web) utilizando comunicación serial entre un ESP32 y un Arduino Nano.

---

## 1. Arquitectura de Hardware y Pines
### Controlador A: Arduino Nano (Actuador Principal y Emisor de Telemetría)
- **Framework:** Arduino / C++
- **Responsabilidad:** Controlar motores, leer entradas analógicas, escuchar comandos seriales desde ESP32 y transmitir telemetría de vuelta.
- **Asignación de Pines:**
  - **Joysticks:** X (Carro) -> A0, Y (Elevación) -> A1, Giro -> A2.
  - **Driver TB6612FNG (Motores DC N20):** 
    - Motor A (Carro): AIN1(D2), AIN2(D4), PWMA(D3).
    - Motor B (Elevación): BIN1(D7), BIN2(D8), PWMB(D5).
    - STBY -> VCC (5V).
  - **Driver DRV8825 (Motor a Pasos Nema 17):** 
    - STEP -> D9, DIR -> D10.
  - **Comunicación:** RX(D0) desde el TX del ESP32, y TX(D1) al RX (GPIO 16) del ESP32 para envío de telemetría.

### Controlador B: ESP32 DevKit V1 (Interfaz Web y Central de Telemetría)
- **Framework:** MicroPython (Thonny IDE)
- **Responsabilidad:** Levantar un servidor web asíncrono, gestionar conexión WiFi, transmitir comandos UART y recopilar/servir telemetría serial.
- **Asignación de Pines:**
  - **UART TX:** GPIO 17 (Conectado a RX del Nano).
  - **UART RX:** GPIO 16 (Conectado a TX del Nano para recepción de datos de telemetría).
  - **LED Status:** GPIO 2.

---

## 2. Requerimientos de Software (Backlog para Agente)

### Tarea 1: Firmware Arduino (main.ino)
- **Lógica de Control Mixto:** Crear una función que sume la intención del Joystick y la intención de la Web.
- **Protocolo Serial:** Implementar un parser simple para comandos UART (Ej: 'F'=Adelante, 'B'=Atrás, 'U'=Subir, 'D'=Bajar, 'L'=Giro Izq, 'R'=Giro Der, 'S'=Stop).
- **Control de Velocidad:** Utilizar PWM para los motores N20.
- **Control Stepper:** Implementar movimiento suave para el Nema 17 usando la librería `AccelStepper` o lógica de retardos no bloqueante.
- **Transmisión de Telemetría:** Implementar envío periódico no bloqueante (cada 250ms usando `millis()`) de tramas seriales JSON conteniendo lecturas de joysticks (`jx`, `jy`, `jg`), velocidad (`sa`, `sb`, `sg`) y dirección (`da`, `db`, `dg`) de los motores.

### Tarea 2: Firmware ESP32 (boot.py y main.py)
- **Menú de Inicio (boot.py):** Integrar un menú interactivo por consola serial con timeout de 5s para iniciar normalmente o ejecutar `sys.exit()` para liberar el REPL de MicroPython.
- **Conexión WiFi:** Implementar función de conexión WiFi robusta.
- **Servidor Web:**
  - Endpoint `/` o `/index.html` que entregue la interfaz web.
  - Endpoint `/telemetria` que devuelva el último estado JSON guardado.
- **Lector UART de Telemetría (main.py):** Iniciar una corrutina asíncrona en segundo plano que reciba y parsee los paquetes seriales JSON desde el Arduino.
- **Transmisión UART:** Al recibir peticiones `/control?cmd=[X]`, enviar el comando correspondiente por UART a 9600 baudios.

### Tarea 3: Interfaz Web (HTML/CSS/JS)
- Diseño responsivo tipo "Control Remoto" con estética glassmorphic premium e industrial.
- Botones táctiles grandes aptos para dispositivos móviles.
- **Dashboard de Telemetría:** Un panel visualizado que realice polling periódico (cada 400ms) a `/telemetria` para actualizar lecturas analógicas de Joysticks y velocidades direccionales en tiempo real.

---

## 3. Instrucciones de Generación para Codex/Antigravity
1. **Archivo 1:** Generar `grúa_arduino.ino` integrando el driver TB6612FNG y AccelStepper.
2. **Archivo 2:** Generar `boot.py` para la configuración de red en MicroPython.
3. **Archivo 3:** Generar `main.py` con el servidor web (uasyncio) y la lógica de envío serial.
4. **Archivo 4:** Generar `index.html` integrado como string dentro de `main.py` o como archivo independiente si el agente lo prefiere.

---

## 4. Consideraciones Técnicas
- **Baudrate:** Configurar ambos dispositivos a 9600 bps.
- **Seguridad:** Los comandos web deben tener un "timeout" de seguridad; si no se recibe un comando de movimiento continuo, los motores deben detenerse.
