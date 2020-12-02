# HADIS - Home Automation, Development and Integration System

<img src="https://github.com/HADIS-Home-Automation/HADIS-Devices/blob/master/assets/logo.png" width="250"/>\
Code for devices of Home Automation, Development and Integration System.

## Introduction

Repository of multiple HADIS supported devices, their code, description and schematics. Devices use Wi-Fi with MQTT protocol to communicate with custom local broker.

## Usage

Upload the code to your device using Serial interface flasher (Arduino IDE guaranteed to work), where you select the correct SOC configurations and proceed to upload.
After a successful upload configure device's WI-FI & MQTT connection settings by long pressing their button for approximately 6 seconds *(devices without a button will
start setup process automatically after about 1 min of not being able to connect)* and connect to their newly created Access Point, named with their MAC address.
In the browser navigate to 10.0.0.1 and fill in the required data in the setup page and click submit. After completing the setup process, device will connect to your
network and is ready to use with HADIS.  

More details about a specific device is available in their respective folders.

Code for controlling HADIS devices is available in [HADIS-Frontend repository](https://github.com/HADIS-Home-Automation/HADIS-Frontend).

## Authors
- Matej Horvat

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](./LICENSE)
