# 🏗️ Proyecto Grúa Torre - Control Maestro

Este proyecto consiste en el control de una grúa torre a escala mediante una arquitectura de doble controlador (ESP32 + Arduino Nano), permitiendo el manejo tanto manual (Joysticks) como remoto (Interfaz Web).

## 🚀 Guía de Inicio Rápido

1. **Arduino Nano:** Cargar el código `arduino/grua_nano.ino`.
2. **ESP32:**
   - Cargar `esp32/boot.py` (configurar WiFi). Al encender, la consola REPL mostrará un menú interactivo con 5s de timeout. Presiona `2` en la terminal para detener el script de inicio y liberar la consola para subir o modificar archivos sin bloqueos.
   - Cargar `esp32/main.py`.
   - Subir `esp32/index.html` (o sincronizar `web/index.html`) a la memoria interna del ESP32.
3. **Uso:** Acceder a la IP del ESP32 desde cualquier navegador para abrir la interfaz web CraneMaster Pro con telemetría en tiempo real.

---

## 🛠️ Arquitectura y Funcionamiento (Paso a Paso)

### 1. Interfaz Web (El Cliente)
La interfaz web es el punto de entrada para el control remoto y la supervisión técnica.
- **Visualizador:** Utiliza imágenes generadas por IA para mostrar qué parte de la grúa se está moviendo.
- **Heartbeat (Latido):** Cuando mantienes presionado un botón, JavaScript envía el comando cada 300ms. Esto es vital porque el sistema tiene un "freno de seguridad" que detiene todo si deja de recibir órdenes.
- **Fetch API:** Se comunica con el ESP32 mediante peticiones HTTP rápidas sin recargar la página.
- **Dashboard de Telemetría:** Consulta cada 400ms el endpoint `/telemetria` del ESP32 para mostrar lecturas de joysticks, velocidad porcentual y dirección en tiempo real de los motores (Carro, Elevación y Giro) con formato de diseño glassmorphism premium.

### 2. ESP32 (El Puente/Servidor)
El ESP32 actúa como un servidor web, traductor de comandos y centralizador de telemetría.
- **Servidor Web Asíncrono:** Escucha las peticiones de la web (ej: `/control?cmd=U`) y sirve el archivo `index.html`.
- **Traductor UART:** Envía los caracteres de control al Arduino Nano de manera serial a 9600 baudios.
- **Receptor UART de Telemetría:** Ejecuta una tarea asíncrona en segundo plano que escucha el bus serial (RX=GPIO16), lee y actualiza localmente las tramas JSON generadas por el Arduino.
- **Endpoint de Telemetría:** Sirve en `/telemetria` el estado más reciente de la grúa en formato JSON para el consumo de la interfaz web con soporte CORS total.
- **LED de Estado:** Parpadea cada vez que procesa una orden.

### 3. Arduino Nano (El Cerebro Motor)
Es el encargado de la potencia, el movimiento físico y la generación de datos de telemetría.
- **Control Mixto:** Revisa de manera no bloqueante los comandos del ESP32 y las lecturas de los Joysticks analógicos (teniendo prioridad el control manual).
- **Generador de Telemetría:** Registra constantemente la posición de los joysticks (ejes X, Y, Giro) y el estado final de velocidad/dirección de los motores.
- **Transmisión Serial:** Cada 250ms, escribe una cadena estructurada en formato JSON hacia el ESP32 para reportar en tiempo real todo el estado del sistema.
- **Drivers:** Controla el TB6612FNG (Motores DC de Carro y Elevación mediante PWM) y el DRV8825 (Motor a pasos del Giro con la librería AccelStepper).
- **Seguridad (Timeout):** Detiene los motores ante pérdidas de comunicación de más de 1 segundo.

---

## 📡 Protocolo de Comandos Seriales

| Carácter | Acción | Dispositivo |
| :--- | :--- | :--- |
| **U** | Subir Carga | Motor B (DC) |
| **D** | Bajar Carga | Motor B (DC) |
| **F** | Carro Adelante | Motor A (DC) |
| **B** | Carro Atrás | Motor A (DC) |
| **L** | Girar Izquierda | Motor Pasos |
| **R** | Girar Derecha | Motor Pasos |
| **S** | STOP (Parada) | Todos |

---

## 📁 Estructura del Código

- `/arduino/grua_nano.ino`: Lógica de motores y lectura de sensores.
- `/esp32/main.py`: Servidor web MicroPython y envío serial.
- `/web/index.html`: Interfaz de usuario premium con lógica de control.
- `/web/assets/`: Imágenes de la construcción y acciones de la grúa.

---
*Desarrollado con ❤️ por el equipo de Antigravity AI.*
