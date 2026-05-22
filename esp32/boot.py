# boot.py -- run on boot-up
import network
import utime as time
import sys
import uselect

# Configuración de red WiFi
SSID = "Tu_Red_WiFi"  # Cambiar por tu SSID
PASSWORD = "Tu_Password" # Cambiar por tu Password

def connect_wifi(ssid=SSID, password=PASSWORD):
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('Conectando a la red WiFi...')
        wlan.connect(ssid, password)
        
        # Esperar hasta que se conecte
        timeout = 10
        while not wlan.isconnected() and timeout > 0:
            time.sleep(1)
            timeout -= 1
            
    if wlan.isconnected():
        print('Configuración de red:', wlan.ifconfig())
    else:
        print('Fallo al conectar a WiFi. Iniciando modo Access Point...')
        setup_ap()

def setup_ap():
    ap = network.WLAN(network.AP_IF)
    ap.config(essid="Grua-Torre-Control", password="password123")
    ap.active(True)
    print('Access Point activo:', ap.ifconfig())

def menu_inicio(timeout_segundos=5):
    """
    Muestra un menú en la terminal. Avanza automáticamente si no hay respuesta.
    """
    print("\n" + "="*40)
    print("      SISTEMA DE CONTROL - GRÚA TORRE")
    print("="*40)
    print("1. Iniciar sistema normalmente (Modo Ejecución)")
    print("2. Detener en modo programación (Liberar REPL)")
    print(f"Selecciona una opción (Avanza a opción 1 en {timeout_segundos}s)...")
    
    # Configurar la terminal para escuchar la entrada del usuario sin bloquear
    poller = uselect.poll()
    poller.register(sys.stdin, uselect.POLLIN)
    
    tiempo_inicio = time.time()
    while (time.time() - tiempo_inicio) < timeout_segundos:
        # Revisar si hay datos en la terminal (espera hasta 100ms por ciclo)
        if poller.poll(100):
            caracter = sys.stdin.read(1)
            if caracter == '1':
                print("\n-> Opción 1 seleccionada. Iniciando...")
                return True
            elif caracter == '2':
                print("\n-> Opción 2 seleccionada. Modo programación activo.")
                print("Consola REPL liberada. Puedes subir o modificar archivos.")
                return False
    
    # Si se agota el tiempo sin respuesta, asumimos que está corriendo en la grúa de forma autónoma
    print("\n-> Tiempo de espera agotado. Iniciando de forma automática...")
    return True

# --- FLUJO DE INICIO ---

# Ejecutamos el menú ANTES de conectar al WiFi o cargar el main
if menu_inicio(timeout_segundos=5):
    # Si elige 1 o se agota el tiempo, conecta a WiFi y avanza a main.py
    connect_wifi(SSID, PASSWORD)
else:
    # Si elige 2, forzamos la detención del script del sistema operativo
    # Esto evita que MicroPython salte automáticamente a ejecutar el main.py
    sys.exit()

