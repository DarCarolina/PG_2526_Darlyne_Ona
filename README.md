# 🏗️ Proyecto Grúa Torre - Control Maestro

Este proyecto consiste en el control de una grúa torre a escala mediante una arquitectura de doble controlador (ESP32 + Arduino Nano), permitiendo el manejo tanto manual (Joysticks) como remoto (Interfaz Web).

## 🚀 Guía de Inicio Rápido

1. **Arduino Nano:** Cargar el código `arduino/grua_nano.ino`.
2. **ESP32:**
   - Cargar `esp32/boot.py` (configurar WiFi).
   - Cargar `esp32/main.py`.
   - Subir `web/index.html` a la memoria interna del ESP32.
3. **Uso:** Acceder a la IP del ESP32 desde cualquier navegador.

---

## 🛠️ Arquitectura y Funcionamiento (Paso a Paso)

### 1. Interfaz Web (El Cliente)
La interfaz web es el punto de entrada para el control remoto.
- **Visualizador:** Utiliza imágenes generadas por IA para mostrar qué parte de la grúa se está moviendo.
- **Heartbeat (Latido):** Cuando mantienes presionado un botón, JavaScript envía el comando cada 300ms. Esto es vital porque el sistema tiene un "freno de seguridad" que detiene todo si deja de recibir órdenes.
- **Fetch API:** Se comunica con el ESP32 mediante peticiones HTTP rápidas sin recargar la página.

### 2. ESP32 (El Puente/Servidor)
El ESP32 actúa como un servidor web y un traductor.
- **Servidor Web Asíncrono:** Escucha las peticiones de la web (ej: `/control?cmd=U`).
- **Traductor UART:** Al recibir un comando HTTP, lo convierte en un solo carácter de texto (ej: 'U') y lo envía físicamente por un cable desde su pin TX al pin RX del Arduino Nano a una velocidad de 9600 baudios.
- **LED de Estado:** Parpadea cada vez que procesa una orden.

### 3. Arduino Nano (El Cerebro Motor)
Es el encargado de la potencia y el movimiento físico.
- **Control Mixto:** En cada vuelta del programa, revisa si hay comandos llegando por el cable serial (del ESP32) O si el usuario está moviendo los Joysticks.
- **Prioridad:** Si el usuario mueve el Joystick, este tiene prioridad sobre el control web.
- **Drivers:**
  - **TB6612FNG:** Controla los motores DC (Carro y Elevación) mediante señales PWM para ajustar la velocidad.
  - **DRV8825:** Controla el motor a pasos del Giro para asegurar movimientos precisos y suaves.
- **Seguridad (Timeout):** Si el Arduino no recibe un comando serial en más de 1 segundo, detiene automáticamente los motores para evitar accidentes por pérdida de conexión WiFi.

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
