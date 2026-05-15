# main.py -- Lógica principal para el ESP32 (Servidor Web y UART)
# Este script levanta un servidor web que recibe comandos HTTP y los traduce a señales UART para el Arduino.

import uasyncio as asyncio
from machine import UART, Pin
import socket

# --- Configuración UART ---
# Usamos el puerto UART 2 del ESP32.
# TX=GPIO17 (Conectar al RX del Arduino Nano)
# RX=GPIO16 (Opcional para recibir datos del Nano)
# Velocidad = 9600 baudios (debe coincidir con el Serial.begin del Nano)
uart = UART(2, baudrate=9600, tx=17, rx=16)

# --- LED de Estado ---
# Usamos el LED integrado en el GPIO 2 para feedback visual.
led = Pin(2, Pin.OUT)

# Función para enviar comandos por UART
def send_command(cmd):
    """Envía un carácter por el puerto UART y hace parpadear el LED."""
    print("Enviando comando UART:", cmd)
    uart.write(cmd.encode()) # Convertimos el string a bytes y enviamos
    
    # Encendemos el LED brevemente para indicar actividad
    led.value(1)
    asyncio.create_task(reset_led())

async def reset_led():
    """Apaga el LED después de 100ms."""
    await asyncio.sleep_ms(100)
    led.value(0)

# --- Manejador de Peticiones HTTP ---
async def handle_client(reader, writer):
    """Gestiona las conexiones entrantes al servidor web."""
    request_line = await reader.readline()
    print("Petición recibida:", request_line)
    
    # Consumimos el resto de las cabeceras HTTP
    while await reader.readline() != b"\r\n":
        pass

    request = str(request_line)
    
    # RUTA: /control?cmd=[X]
    # Se usa para enviar comandos de movimiento desde la web sin recargar.
    if "GET /control?cmd=" in request:
        # Extraemos el carácter del comando (está después de 'cmd=')
        start = request.find("cmd=") + 4
        cmd = request[start:start+1]
        
        # Validamos que sea un comando conocido antes de enviarlo por UART
        if cmd in ["F", "B", "L", "R", "U", "D", "S"]:
            send_command(cmd)
            response = '{"status": "success", "msg": "Comando ' + cmd + ' enviado"}'
        else:
            response = '{"status": "error", "msg": "Comando invalido"}'
            
        # Enviamos cabeceras HTTP con soporte para CORS (permitir peticiones de otros dominios)
        writer.write('HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n')
        writer.write(response)
    
    # RUTA: / o /index.html
    # Sirve la interfaz web principal desde la memoria del ESP32.
    elif "GET / " in request or "GET /index.html" in request:
        try:
            with open('index.html', 'r') as f:
                content = f.read()
            writer.write('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n')
            writer.write(content)
        except:
            writer.write('HTTP/1.1 404 Not Found\r\n\r\nArchivo index.html no encontrado en el ESP32.')
    
    else:
        # Ruta no encontrada
        writer.write('HTTP/1.1 404 Not Found\r\n\r\n')

    # Cerramos la conexión de forma limpia
    await writer.drain()
    writer.close()
    await writer.wait_closed()

async def main():
    """Función principal para iniciar el servidor."""
    print("Iniciando servidor web asíncrono en puerto 80...")
    # Escuchamos en todas las interfaces (0.0.0.0)
    server = await asyncio.start_server(handle_client, "0.0.0.0", 80)
    
    # Bucle infinito para mantener el servidor corriendo
    while True:
        await asyncio.sleep(1)

# Punto de entrada de la aplicación
try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Servidor detenido por el usuario.")
except Exception as e:
    print("Error fatal:", e)
