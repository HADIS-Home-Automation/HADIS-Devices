# HADIS - RGBW

MQTT RGBW LED controller.
Setup mode starts automatically after 1 minute without connection to MQTT broker. During this time blue LED is turned on the controller.

## Electronics
* Device: MagicHome RGBW controller

## Pins
* Red LED: GPIO 12
* Blue LED: GPIO 5
* Green LED: GPIO 13
* White LED: GPIO 15

## MQTT topics
* HADIS/*deviceName*/RGBW -> handle RGBW brightness level
* HADIS/*deviceName*/SETUP -> handle setup mode activation
* HADIS/*deviceName*/STATUS -> handle device status (ONLINE/OFFLINE)

*deviceName* is a placeholder for specific topics

## Pictures

![RGBW front](./assets/Thumb/RGBW-Front.jpg)

Controller with case

![RGBW inside](./assets/Thumb/RGBW-Inside.jpg)

Controller without case
