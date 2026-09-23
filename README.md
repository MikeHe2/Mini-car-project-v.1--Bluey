# Mochan – Mini Car Project v.1

Mini robot móvil basado en ESP32-C3 con control por WiFi (punto de acceso propio), pantalla OLED con "cara" animada (ojos estilo robot) y modos de movimiento aleatorio tipo mascota.

## Hardware

- **Microcontrolador:** ESP32-C3 DevKitM-1
- **Pantalla:** OLED SSD1306 128x64, I2C (dirección `0x3C`)
  - SDA → GPIO8
  - SCL → GPIO9
- **Driver de motores:** puente H con pines de dirección + STBY (control on/off, sin PWM — velocidad fija al máximo)
  - `LF` → GPIO0 (motor izquierdo, adelante)
  - `LB` → GPIO1 (motor izquierdo, atrás)
  - `RF` → GPIO2 (motor derecho, adelante)
  - `RB` → GPIO3 (motor derecho, atrás)
  - `STBY` → GPIO10
- **Alimentación:** 2 baterías de 3.7V (Li-ion/LiPo). Importante mantener buena conexión de GND compartido entre baterías, driver, ESP32 y pantalla — un GND flojo puede causar que la pantalla no encienda o el sistema se comporte de forma errática.
- **Switch de encendido:** interruptor físico en línea con la alimentación de las baterías, para encender/apagar el carro completo.

## Funcionalidad

- **Control remoto por WiFi:** el ESP32 crea su propia red WiFi (`softAP`, SSID: `Bluey`) con un servidor web y DNS captivo. Al conectarse, se puede abrir un panel de control desde el navegador (botones: adelante, atrás, izquierda, derecha, stop).
- **Cara animada:** usa la librería [FluxGarage RoboEyes](https://www.fluxgarage.com) para dibujar ojos animados en el OLED, con parpadeo automático e "idle mode".
- **Modos de comportamiento aleatorio** (cuando no hay control manual activo):
  - `SLEEP` (`RANDOM_OFF`): sin movimientos aleatorios.
  - `WIGGLE` (`RANDOM_SOFT`): movimientos aleatorios suaves y poco frecuentes.
  - `CURIOUS` (`RANDOM_NORMAL`, modo por defecto): movimientos aleatorios más activos.
- **Diagnóstico I2C:** si la pantalla no se detecta al arrancar, el firmware escanea el bus I2C y reporta por Serial las direcciones encontradas (útil para depurar cableado SDA/SCL/GND).

## Estructura del proyecto

```
src/
  mochan.cpp              # Firmware principal (motores, WiFi, servidor web, OLED)
  FluxGarage_RoboEyes.h   # Librería de animación de ojos (Dennis Hoelscher / FluxGarage)
platformio.ini            # Configuración de PlatformIO (board, framework, dependencias)
```

## Dependencias

Gestionadas por PlatformIO (`platformio.ini`):

- `adafruit/Adafruit SSD1306`
- `adafruit/Adafruit GFX Library`
- FluxGarage RoboEyes (incluida directamente en `src/`)

## Compilar y cargar

Con [PlatformIO](https://platformio.org/) instalado (CLI o extensión de VS Code):

```bash
pio run                # compilar
pio run --target upload  # compilar y cargar al ESP32-C3
pio device monitor     # ver logs por Serial (115200 baud)
```

## Uso

1. Encender el robot con el switch de alimentación (baterías conectadas, GND firme entre todos los componentes).
2. Conectarse desde un celular/laptop a la red WiFi `Bluey`.
3. Se debería abrir automáticamente el panel de control (portal cautivo) o navegar manualmente a la IP del AP (por defecto `192.168.4.1`).
4. Usar los botones direccionales para mover el robot manualmente, o elegir un modo (`SLEEP` / `WIGGLE` / `CURIOUS`) para que se mueva solo.

## Notas de hardware conocidas

- El control de motores es **on/off puro** (sin PWM), por lo que ambos motores giran siempre a máxima potencia cuando están activos.
- Si un motor gira libre en el aire pero pierde fuerza o no se mueve apoyado en el suelo, es señal de falta de torque bajo carga — revisar caja de reducción (engranes), estado del canal del driver, o el motor mismo, antes de sospechar del código (la lógica de control es idéntica para ambos motores).

## Créditos

- Animación de ojos: [FluxGarage RoboEyes](https://www.fluxgarage.com) — Dennis Hoelscher (licencia GPLv3, ver cabecera en `FluxGarage_RoboEyes.h`).
- Proyecto base / inspiración: canal de YouTube "Huy Vector" (referenciado en la interfaz web).
