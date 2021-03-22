# HADIS - Solo

MQTT relay switch.
Start setup mode by pressing the button for approximately 6sec.

## Electronics
* Device: [Sonoff Basic R2](https://sonoff.tech/product/wifi-diy-smart-switches/basicr2)

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

![Solo front](./assets/Thumb/Solo-Front.jpg)

Switch with case

![Solo inside](./assets/Thumb/Solo-Inside.jpg)

Switch without case
