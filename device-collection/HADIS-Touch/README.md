# HADIS - Touch

MQTT relay wall touch switch for light control.
Start setup mode by pressing the touch button for approximately 6sec.

## Electronics
* Device: [Sonoff Touch TXT3](https://www.itead.cc/smart-home/sonoff-tx-series-wifi-smart-wall-switches.html)

## Pins
* Button: GPIO 0
* Relay: GPIO 12
* Indicator LED: GPIO 13

## MQTT topics
* HADIS/*deviceName*/SWITCH -> handle relay state
* HADIS/*deviceName*/SETUP -> handle setup mode activation
* HADIS/*deviceName*/STATUS -> handle device status (ONLINE/OFFLINE)

*deviceName* is a placeholder for specific topics

## Pictures

![Touch front](./assets/Thumb/Touch-Front.jpg)

Switch with cover plate

![Touch inside](./assets/Thumb/Touch-Inside.jpg)

Switch without cover plate

![Touch back](./assets/Thumb/Touch-Back.jpg)

Switch from the back
