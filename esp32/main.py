# main.py -- principal logic for ESP32
import uasyncio as asyncio
from machine import UART, Pin
import socket

# Configuración UART (TX=GPIO17, RX=GPIO16, Baud=9600)
# Conectado al RX del Arduino Nano
uart = UART(2, baudrate=9600, tx=17, rx=16)

# LED de estado (GPIO 2)
led = Pin(2, Pin.OUT)

# Función para enviar comandos por UART
def send_command(cmd):
    print("Enviando comando UART:", cmd)
    uart.write(cmd.encode())
    # Parpadeo de LED para indicar actividad
    led.value(1)
    asyncio.create_task(reset_led())

async def reset_led():
    await asyncio.sleep_ms(100)
    led.value(0)

# Manejador de peticiones HTTP
async def handle_client(reader, writer):
    request_line = await reader.readline()
    print("Petición recibida:", request_line)
    
    # Leer el resto de la cabecera
    while await reader.readline() != b"\r\n":
        pass

    request = str(request_line)
    
    # Lógica de ruteo
    if "GET /control?cmd=" in request:
        # Extraer el comando del query string
        start = request.find("cmd=") + 4
        cmd = request[start:start+1]
        
        # Validar y enviar
        if cmd in ["F", "B", "L", "R", "U", "D", "S"]:
            send_command(cmd)
            response = '{"status": "success", "msg": "Comando ' + cmd + ' enviado"}'
        else:
            response = '{"status": "error", "msg": "Comando invalido"}'
            
        writer.write('HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n\r\n')
        writer.write(response)
    
    elif "GET / " in request or "GET /index.html" in request:
        # Servir la interfaz web (se puede leer de un archivo o enviar como string)
        try:
            with open('index.html', 'r') as f:
                content = f.read()
            writer.write('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n')
            writer.write(content)
        except:
            writer.write('HTTP/1.1 404 Not Found\r\n\r\nFile Not Found')
    
    else:
        writer.write('HTTP/1.1 404 Not Found\r\n\r\n')

    await writer.drain()
    writer.close()
    await writer.wait_closed()

async def main():
    print("Iniciando servidor web asíncrono...")
    server = await asyncio.start_server(handle_client, "0.0.0.0", 80)
    
    # Bucle principal
    while True:
        await asyncio.sleep(1)

# Iniciar aplicación
try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Servidor detenido.")
