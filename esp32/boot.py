# boot.py -- run on boot-up
import network
import utime

# Configuración de red WiFi
SSID = "Tu_Red_WiFi"  # Cambiar por tu SSID
PASSWORD = "Tu_Password" # Cambiar por tu Password

def connect_wifi():
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    if not wlan.isconnected():
        print('Conectando a la red WiFi...')
        wlan.connect(SSID, PASSWORD)
        
        # Esperar hasta que se conecte
        timeout = 10
        while not wlan.isconnected() and timeout > 0:
            utime.sleep(1)
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

connect_wifi()
