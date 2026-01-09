# Boiler controller
This project has born to control temperature and pressure of my home boiler.
It make use of a TE M32JM-000105-100PG (from now on M32JM) pressure trasducer (which include a temperature trasducer).
This version is an i2c device. I've selected the i2c version to avoid reading over the digital input. Bad choice, likely.

## Detail over the device
The same code used in the M32JM datasheet can be found in use for several other similar devices.
Due to that, basic sketches has been found in Arduino playground. I've selected one, and built on top of it.

In general, regardingless of the warning of the datasheet, performing a Wire.read(4) (give me 4 bytes) is working properly.

## Wiring
### M32JM specific
M32JM-000105-100PG specifically require a pullup (from the technical specifications, 4.7KOhm should be OK) from SCL to VCC and from SDA to VCC.

Custom Board Schema
```plaintext
       VCC (+5V)
         |
         +----[4.7kΩ]----> SDA
         |
         +----[4.7kΩ]----> SCL
         |
        ---
       |   |
       |   | (GND)
        ---
```

### Wiring
i2c is not meant for long distance.
M32JM blocked Arduino Uno more than once.
Boiler pump start was breaking up the i2c connection as well.
Arduino R4 Minima seems to perform better.

I have decided to keep the Arduino device away from the M32JM, running i2c over a Ethernet cable.

2 adafruit devices has been added into the chain:
* [Adafruit TCA4307 Hot-Swap I2C Buffer with Stuck Bus Recovery]()
* [Adafruit LTC4311 I2C Extender / Active Terminator]()

The full chain is:
`Arduino -> TCA4307 -> LTC4311 -> RJ45 (long distance) -> Custom Board -> probe`

TCA4307 RDY (ready) pin is connected to pin 12 (purple)
TCA4307 EN (enable) pin is connected to pin 13 (gray)
LTC4311 EN is NOT connected

### Logical function
1) Arduino startup
2) Watchdog is created
3)

### Caveats and workaround
When the device was reporting a "stale" read (check the datasheet) it's always stale from that moment on.
It's not a matter of reading frequency.
So based on another portion of code, when stale is detected, another i2c read with 0 bytes is performed.

The casting in the conversion is needed as otherwise (beginner hint) the variables are managed as `int8_t` (or Arduino int), so max int 128 (AFAIR).

### reference
[original code](https://forum.arduino.cc/t/ms-4525do/298848/19)
[TE datasheet](https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FM3200%7FA11%7Fpdf%7FEnglish%7FENG_DS_M3200_A11.pdf%7F20010080-00)

## Devices
### TCA4307
https://www.adafruit.com/product/5159 Adafruit TCA4307 Hot-Swap I2C Buffer with Stuck Bus Recovery
https://cdn-learn.adafruit.com/assets/assets/000/103/421/original/tca4307.pdf
https://www.ti.com/lit/ds/symlink/tca4307.pdf

ENABLE pin (NOT in use):
Active-high chip enable pin. If EN is low, the TCA4307 is in a low current mode. It also
disables the rise-time accelerators, disables the bus pre-charge circuitry (after the initial
power up), drives READY low, isolates SDAIN from SDAOUT and isolates SCLIN from
SCLOUT. EN should be high (at VCC) for normal operation. Connect EN to VCC if this
feature is not being used.

READY pin:
Connection flag/rise-time accelerator control. Ready is low when either EN is low or the
start-up sequence has not been completed. READY goes high when EN is high and start-up
is complete. Connect a 10-kΩ resistor from this pin to VCC to provide the pull-up current.

### LTC4311
https://www.adafruit.com/product/4756 Adafruit LTC4311 I2C Extender / Active Terminator
https://www.analog.com/media/en/technical-documentation/data-sheets/4311fa.pdf

ENABLE pin
ENABLE: Device Enable Input. This is a 1V nominal digital
threshold input pin. For normal operation drive ENABLE
to a voltage greater than 1.5V. Driving ENABLE below the
0.4V threshold puts the device in a low (<5μA) current
shutdown mode and puts the BUS pins in a high impedance state. If unused, connect to VCC

### RJ45 optimization
https://electronics.stackexchange.com/questions/107663/sending-i2c-reliabily-over-cat5-cables
https://pinoutguide.com/Net/poe_pinout.shtml

Suggested
Pin 1 (Pair 1, White/Orange): SCL+
Pin 2 (Pair 1, Orange): SCL- (GND)
Pin 3 (Pair 2, White/Green): SDA+
Pin 4 (Pair 3, Blue): +12V
Pin 5 (Pair 3, White/Blue): +12V
Pin 6 (Pair 2, Green): SDA- (GND)
Pin 7 (Pair 4, White/Brown): GND
Pin 8 (Pair 4, Brown): GND

## instuctions
The following libraries are needed (using arduino-cli, feel free to pick the library names)
* `arduino-cli lib install MQTT`
* `arduino-cli lib install Ethernet`
* `arduino-cli lib install ArduinoJson`

You need to adjust the `#define MQTTBROKER` with your FQDN to be used.

### Compile instructions
This is mainly how to compile it for the specific arduino used in this project. Feel free to adapt it.
`arduino-cli compile boiler-smd.ino -j0 -b arduino:renesas_uno:minima && ex=1 ; while [[ $ex -ne 0 ]] ; do arduino-cli upload boiler-smd.ino --port=/dev/serial/by-id/usb-Arduino_UNO_R4_Minima_330A131459313835070F33324B572E41-if00 -b arduino:renesas_uno:minima ; ex=$? ; done  && screen -L -Logfile ~/logs/arduino-$(date +%Y%m%d-%H%M%S).log /dev/serial/by-id/usb-Arduino_UNO_R4_Minima_330A131459313835070F33324B572E41-if00 115200`
