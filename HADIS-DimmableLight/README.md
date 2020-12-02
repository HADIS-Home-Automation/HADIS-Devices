# HADIS - Dimmable Light

MQTT controller for ikea LED lamp.
Start setup mode by pressing encoder button for approximately 6sec.

## Electronics
* Microcontroller: [LOLIN D1 mini](https://www.wemos.cc/en/latest/d1/d1_mini.html)
* Mosfet: [IRLZ44NPbF](http://www.infineon.com/dgdl/irlz44npbf.pdf?fileId=5546d462533600a40153567217c32725)
* Rotary Encoder
* 10k ohm resistor

Full schematic available [here](./assets/DimmableLightSchematic.pdf).

## Pins
* Mosfet: GPIO 5
* Encoder S1: GPIO 12
* Encoder S2: GPIO 14
* Encoder button: GPIO 13

## Pictures

<img src="https://github.com/HADIS-Home-Automation/HADIS-Devices/blob/master/HADIS-DimmableLight/assets/DimmableLight-Case.jpg" width="500"/>

Finished controller

<img src="https://github.com/HADIS-Home-Automation/HADIS-Devices/blob/master/HADIS-DimmableLight/assets/DimmableLight-Side.jpg" width="500"/>

Side of the controller's insides

<img src="https://github.com/HADIS-Home-Automation/HADIS-Devices/blob/master/HADIS-DimmableLight/assets/DimmableLight-Wiring.jpg" width="500"/>

Controller's wiring on the back side

<img src="https://github.com/HADIS-Home-Automation/HADIS-Devices/blob/master/HADIS-DimmableLight/assets/DimmableLight-Back.jpg" width="500"/>

Controller with D1 mini attached
