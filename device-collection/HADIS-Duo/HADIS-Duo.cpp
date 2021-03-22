#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define TRUE (1 == 1)
#define FALSE (!TRUE)
#define RESET 0

// ------------------------------------------------------------------
// SERIAL COMMUNICATION DEFINITIONS    SERIAL COMMUNICATION DEFINITIONS
// ------------------------------------------------------------------

// remove comment for debugging
// #define DEBUG

// serial interface enabled if DEBUG is active
#ifdef DEBUG
#define SERIAL_BEGIN(x) (Serial.begin(x))
#define PRINTLN(x) (Serial.println(x))
#define PRINT(x) (Serial.print(x))
#else
#define SERIAL_BEGIN(x)
#define PRINTLN(x)
#define PRINT(x)
#endif


// ------------------------------------------------------------------
// FUNCTION DECLARATIONS    FUNCTION DECLARATIONS    FUNCTION DECLARATIONS
// ------------------------------------------------------------------
void setupServer();

void hostAP();

void generateTopics();

void shortPressPoll();


// ------------------------------------------------------------------
// PINS    PINS    PINS    PINS    PINS    PINS    PINS    PINS
// ------------------------------------------------------------------
// used pins of microcontroller definitions
const int pinButton1 = 0;
const int pinButton2 = 9;
const int RELAY1 = 12;
const int RELAY2 = 5;
const int LED_indicator = 13; //only used for AP indicator


// ------------------------------------------------------------------
// GLOBAL VARIABLES    GLOBAL VARIABLES     GLOBAL VARIABLES
// ------------------------------------------------------------------

// button press variables
#define SHORT_PRESS_DURATION_1 5
#define LONG_PRESS_DURATION_1 500
#define SHORT_PRESS_DURATION_2 5
#define LONG_PRESS_DURATION_2 100

volatile int longPressB1;
volatile int longPressB1Finished = FALSE;
volatile int shortPressB1;
volatile int longPressB2;
volatile int longPressB2Finished = FALSE;
volatile int shortPressB2;

// relay1 variables
volatile int relay1State = 0;
volatile int hardwareTrigger1 = FALSE;

// relay2 variables
volatile int relay2State = 0;
volatile int hardwareTrigger2 = FALSE;

// AP hosting variables
char macAddress[18];
char ssidAP[32];
const int AP_HOST_TIMEOUT = 300000;
const int AP_INPUT_RECEIVED_DELAY = 5000;
unsigned long inputReceivedTime = RESET;

// MQTT variables
char topicSwitch1[90];
char topicSwitch2[90];
char topicLong2[90];
char topicSetup[90];
char topicStatus[90];
char msg[50];
const int CONNECTION_RETRY_DELAY = 10000;

// Library variables
ESP8266WebServer server(80);
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);


// ------------------------------------------------------------------
// INPUT DEFINITIONS   INPUT DEFINITIONS    INPUT DEFINITIONS
// ------------------------------------------------------------------

// struct for reading & storing device settings to EEPROM
typedef struct EepromStorageStruct {
    char ssid[32] = "DefaultSSID";
    char password[65] = "DefaultPass";
    char deviceName[41] = "DefaultName";
    byte mqtt[4] = {0, 0, 0, 0};

} EepromStorage;

EepromStorage deviceSettings;

// char arrays for storing setting data from Access Point input
char inputSSID[32] = "DefaultSSID";
char inputPass[65] = "DefaultPass";
char inputDeviceName[41] = "DefaultName";
byte inputMqtt[4];


// ---------------------------------------------------------------------------------------------------------------------------------
// PROGRAM CODE START    PROGRAM CODE START    PROGRAM CODE START    PROGRAM CODE START    PROGRAM CODE START    PROGRAM CODE START
// ---------------------------------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------
// BUTTON INTERRUPT    BUTTON INTERRUPT    BUTTON INTERRUPT
// ------------------------------------------------------------------

// Timer ISR, detects short & long press by counting time of button pressed
void IRAM_ATTR buttonISR() {

    // Local variables
    static int shortPressedFlagB1 = FALSE;
    static int shortPressedFlagB2 = FALSE;
    static int counterB1 = 0;
    static int counterB2 = 0;

    // Button1 touch detected
    if (!digitalRead(pinButton1)) {

        // if long press B1 already triggered, skip function
        if(longPressB1Finished)
            return;

        counterB1++;

        // Short press
        if (counterB1 > SHORT_PRESS_DURATION_1) {
            shortPressedFlagB1 = TRUE;

            // Long press
            if (counterB1 > LONG_PRESS_DURATION_1) {
                shortPressedFlagB1 = FALSE;
                longPressB1 = TRUE;
            }
        }
    }
    // Button1 no touch detected
    else {

        // Check if short pressed
        if (shortPressedFlagB1) {
            shortPressB1 = TRUE;
        }

        shortPressedFlagB1 = FALSE;
        counterB1 = RESET;
    }

    // Button2 touch detected
    if (!digitalRead(pinButton2)) {

        // if long press B2 already triggered, skip function
        if(longPressB2Finished)
            return;

        counterB2++;

        // Short press
        if (counterB2 > SHORT_PRESS_DURATION_2) {
            shortPressedFlagB2 = TRUE;

            // Long press
            if (counterB2 > LONG_PRESS_DURATION_2) {
                shortPressedFlagB2 = FALSE;
                longPressB2 = TRUE;
            }
        }
    }
    // Button2 no touch detected
    else {

        // Check if short pressed
        if (shortPressedFlagB2) {
            shortPressB2 = TRUE;
        }

        shortPressedFlagB2 = FALSE;
        counterB2 = RESET;
    }

}

// ------------------------------------------------------------------
// HANDLE SERVER    HANDLE SERVER     HANDLE SERVER     HANDLE SERVER
// ------------------------------------------------------------------

// main web page for settings entry
void handleRoot() {
    // html setup page, compressed with CyberChef tool (https://gchq.github.io/CyberChef/) using Gzip, To Hex & Split
#define SETUP_HTML_LENGTH 1078
    const uint8_t setupHtml[] = {0x1f, 0x8b, 0x08, 0x02, 0xde, 0xcd, 0xb7, 0x5f, 0x00, 0xff, 0x0b, 0x87, 0xbd, 0x57,
                                 0x7b, 0x6e, 0xe3, 0xb6, 0x13, 0xfe, 0x3f, 0xa7, 0x98, 0xd5, 0xa2, 0xc0, 0x06, 0x88,
                                 0xe3, 0x97, 0xe2, 0x76, 0x5d, 0x45, 0xc0, 0xfe, 0x7e, 0xe9, 0x62, 0x03, 0x34, 0xc5,
                                 0x16, 0x49, 0x0f, 0x40, 0x49, 0x94, 0xc5, 0x86, 0x22, 0x15, 0x6a, 0x94, 0xc4, 0x2d,
                                 0x7a, 0x8d, 0x5e, 0xa4, 0x37, 0xea, 0x49, 0x76, 0xa8, 0x47, 0x22, 0xca, 0xb2, 0x9b,
                                 0x06, 0xc5, 0xca, 0x96, 0x6d, 0xcd, 0x68, 0x86, 0xdf, 0xbc, 0x3e, 0xca, 0x41, 0x86,
                                 0xb9, 0x0c, 0x8f, 0x82, 0x8c, 0xb3, 0x24, 0x3c, 0x02, 0x3a, 0x82, 0x9c, 0x23, 0x83,
                                 0x38, 0x63, 0xa6, 0xe4, 0x78, 0xee, 0x55, 0x98, 0x4e, 0xbe, 0xf3, 0x5a, 0x15, 0x0a,
                                 0x94, 0x3c, 0xfc, 0xf4, 0xf7, 0x9f, 0x7f, 0x5d, 0x5c, 0x5e, 0x4f, 0xae, 0x39, 0x56,
                                 0x45, 0x30, 0x6d, 0x84, 0xcd, 0x0d, 0x25, 0x6e, 0xbb, 0xdf, 0xf6, 0x78, 0x8b, 0xba,
                                 0x80, 0xdf, 0x9f, 0x2e, 0xed, 0x11, 0xb1, 0xf8, 0x76, 0x63, 0x74, 0xa5, 0x92, 0x35,
                                 0xbc, 0x9d, 0xbf, 0xff, 0x76, 0x75, 0xb1, 0xf8, 0xde, 0xb9, 0xa1, 0x60, 0x49, 0x22,
                                 0xd4, 0x66, 0x0d, 0x33, 0x57, 0x9e, 0x33, 0xb3, 0x11, 0x6a, 0x47, 0x1c, 0x6b, 0xa9,
                                 0xcd, 0x1a, 0x1e, 0x32, 0x81, 0xdc, 0xd5, 0x44, 0xfa, 0x71, 0x52, 0x66, 0x2c, 0xd1,
                                 0x0f, 0x64, 0x44, 0xaf, 0xf9, 0xac, 0x78, 0xa4, 0x2f, 0xb3, 0x89, 0xd8, 0xbb, 0xd9,
                                 0x09, 0xb4, 0xef, 0xd3, 0xc5, 0x31, 0x7d, 0xc2, 0x82, 0x74, 0xf6, 0x1c, 0xd1, 0xfb,
                                 0xc7, 0xae, 0xdf, 0x54, 0x2b, 0x9c, 0x94, 0xe2, 0x37, 0xbe, 0x86, 0x05, 0xb9, 0x7c,
                                 0x56, 0xfe, 0x71, 0xe4, 0x06, 0x2e, 0xc5, 0x20, 0xf6, 0x44, 0x94, 0x85, 0x64, 0xdb,
                                 0x35, 0x08, 0x25, 0x85, 0xe2, 0x93, 0x48, 0xea, 0xf8, 0x76, 0xd4, 0x3e, 0xd2, 0xc9,
                                 0x76, 0x60, 0xbc, 0x27, 0xfe, 0x1a, 0x4d, 0xca, 0x72, 0x21, 0xc9, 0x2f, 0x33, 0x82,
                                 0xc9, 0x13, 0x28, 0xb9, 0x11, 0xa9, 0x7b, 0x1b, 0xf2, 0x47, 0x9c, 0x30, 0x29, 0x36,
                                 0xe4, 0x21, 0xe6, 0x0a, 0xb9, 0x19, 0x24, 0xab, 0x5f, 0x96, 0xc5, 0xfc, 0xfd, 0xea,
                                 0xe3, 0x72, 0x14, 0x58, 0xe6, 0x0f, 0x60, 0xb5, 0xf9, 0xdf, 0xb1, 0x39, 0x00, 0x79,
                                 0x90, 0x86, 0xf1, 0x04, 0xc6, 0xcc, 0x24, 0x83, 0xa5, 0x1e, 0x44, 0x82, 0xd9, 0x1a,
                                 0x56, 0x33, 0x27, 0xeb, 0x4e, 0xcf, 0xac, 0x76, 0x54, 0x1d, 0x06, 0x56, 0xa1, 0x1e,
                                 0x5f, 0x49, 0xa8, 0x61, 0x8f, 0x16, 0xba, 0x14, 0x28, 0x34, 0x59, 0x19, 0x2e, 0x19,
                                 0x8a, 0xfb, 0x61, 0x67, 0xf5, 0x93, 0xf5, 0xb1, 0x3e, 0x86, 0x8b, 0x3e, 0x4e, 0x5e,
                                 0x80, 0xd6, 0xb6, 0x0f, 0xf8, 0x67, 0x43, 0xfd, 0xb0, 0x71, 0xeb, 0xdb, 0xf6, 0x35,
                                 0x2e, 0x59, 0xd7, 0xe7, 0x0b, 0x1a, 0xb7, 0xdf, 0x03, 0x92, 0xa7, 0xb8, 0x3f, 0x1d,
                                 0x74, 0x56, 0x78, 0x68, 0x70, 0xd3, 0x85, 0x7d, 0xb9, 0xee, 0xdb, 0x80, 0xe7, 0xb3,
                                 0xd9, 0x37, 0xc3, 0x78, 0x4c, 0xc2, 0xa9, 0x47, 0xf6, 0x0d, 0xb4, 0x9d, 0xcd, 0x9d,
                                 0x24, 0x3c, 0x25, 0x69, 0x57, 0xd5, 0x1b, 0xc0, 0xb9, 0xbf, 0x6f, 0x00, 0x6d, 0x18,
                                 0xa7, 0xf9, 0x1d, 0xe2, 0x8f, 0xd4, 0x62, 0xfb, 0xe6, 0x30, 0x95, 0xfc, 0x45, 0xe6,
                                 0x65, 0xc1, 0xd4, 0xc0, 0x47, 0x0f, 0xc4, 0x72, 0x7f, 0x85, 0xe7, 0x96, 0x53, 0x96,
                                 0x75, 0x79, 0xe8, 0xf3, 0x1f, 0x96, 0xfa, 0x3f, 0x97, 0xf2, 0x85, 0x43, 0x4f, 0xb8,
                                 0xc9, 0xf9, 0x9e, 0xb1, 0xb2, 0x5a, 0x78, 0x23, 0xf2, 0x42, 0x1b, 0x64, 0x0a, 0x47,
                                 0xf9, 0xa5, 0x42, 0xd4, 0xc3, 0x90, 0x0e, 0x72, 0xc0, 0x6b, 0x2a, 0x7c, 0xa0, 0x86,
                                 0x1d, 0x6d, 0x8c, 0x4d, 0xcf, 0x4b, 0xea, 0xdb, 0x04, 0xb0, 0xce, 0xf4, 0x3d, 0x37,
                                 0x27, 0xdd, 0x15, 0x8b, 0xed, 0xac, 0x3e, 0x5d, 0xa6, 0x3a, 0xae, 0xca, 0x7f, 0xb3,
                                 0xfd, 0xf4, 0xfc, 0x0f, 0x29, 0xa1, 0x3f, 0x3b, 0xbf, 0x56, 0x25, 0x8a, 0x74, 0x3b,
                                 0x6a, 0x77, 0xb8, 0x53, 0x46, 0xc3, 0x09, 0xa6, 0xed, 0xae, 0x19, 0x4c, 0x9b, 0x5d,
                                 0x38, 0xb0, 0xec, 0x4f, 0x5f, 0x89, 0xb8, 0x07, 0x91, 0x9c, 0x7b, 0xb4, 0x97, 0x74,
                                 0x1b, 0xb0, 0x14, 0x61, 0x90, 0x2d, 0x42, 0x68, 0xb7, 0x60, 0x98, 0x40, 0xbb, 0x09,
                                 0x93, 0x30, 0x98, 0x92, 0x96, 0x9c, 0x90, 0x59, 0xcf, 0xd8, 0xf2, 0x28, 0x59, 0x37,
                                 0xe6, 0x9d, 0x90, 0x5a, 0xae, 0x93, 0xd5, 0xf2, 0x22, 0xbc, 0x54, 0xb4, 0x69, 0x20,
                                 0x04, 0x51, 0x78, 0x7d, 0x7d, 0x79, 0x11, 0x4c, 0xa3, 0x10, 0x98, 0x4a, 0xec, 0xf5,
                                 0x67, 0x56, 0x96, 0x0f, 0x54, 0xdf, 0x5a, 0xa6, 0x53, 0xd8, 0xea, 0xca, 0x40, 0xa6,
                                 0x73, 0x0e, 0x8a, 0x23, 0x29, 0x6e, 0x01, 0x35, 0x95, 0x53, 0x29, 0x1e, 0x23, 0x60,
                                 0x26, 0x4a, 0x48, 0xf8, 0xbd, 0x88, 0xb9, 0x15, 0xd7, 0xf7, 0x3e, 0x88, 0x54, 0x9c,
                                 0x3a, 0x19, 0x69, 0x17, 0xc3, 0x8c, 0xc3, 0xe5, 0x67, 0xa0, 0x26, 0x31, 0xbc, 0x2c,
                                 0x9f, 0x7c, 0x47, 0x46, 0xdf, 0x72, 0x43, 0x1c, 0x54, 0xdf, 0x70, 0xf5, 0xf3, 0xcd,
                                 0x0d, 0xa4, 0x82, 0xcb, 0xa4, 0x3c, 0x0d, 0xa6, 0x45, 0x1f, 0x75, 0xaa, 0x4d, 0x0e,
                                 0xb6, 0xe6, 0x5a, 0xd5, 0x21, 0x55, 0xe8, 0x01, 0x3d, 0xbc, 0x64, 0x9a, 0x22, 0x24,
                                 0x0a, 0xc7, 0x7e, 0x88, 0xb5, 0x41, 0xe6, 0xd7, 0xd1, 0xad, 0x29, 0x5d, 0x7e, 0x48,
                                 0x0f, 0x2b, 0x54, 0xab, 0xf0, 0xdd, 0x72, 0x5e, 0x3f, 0xec, 0x1c, 0x53, 0x19, 0xec,
                                 0xb5, 0x6b, 0xd1, 0x10, 0x21, 0x6e, 0x0b, 0x4e, 0x65, 0xa0, 0x16, 0xa0, 0x05, 0xd8,
                                 0xa3, 0xe4, 0x6a, 0x83, 0xd9, 0xb9, 0xb7, 0x9c, 0x7b, 0xa0, 0x58, 0x4e, 0x2a, 0xeb,
                                 0xd5, 0x03, 0x9a, 0xbe, 0x98, 0x67, 0x5a, 0xd2, 0x28, 0x74, 0x22, 0xc3, 0xef, 0x2a,
                                 0x61, 0x38, 0x15, 0x75, 0x07, 0x49, 0x97, 0x57, 0x17, 0xcd, 0xca, 0x7f, 0x2d, 0x9a,
                                 0x95, 0xdf, 0xa1, 0xe9, 0x3c, 0x0f, 0x10, 0x3d, 0x8b, 0x0f, 0xa0, 0xb2, 0xe9, 0x6e,
                                 0x10, 0xb9, 0x2a, 0xdb, 0x39, 0xb1, 0x24, 0x0f, 0xe7, 0x5e, 0xc7, 0x8a, 0x6d, 0x4f,
                                 0x8e, 0x20, 0xec, 0xdd, 0x68, 0x39, 0xcd, 0x6b, 0x21, 0xab, 0x2a, 0x8f, 0xb8, 0x21,
                                 0xd0, 0x82, 0xea, 0x35, 0xab, 0xc1, 0x9f, 0x7b, 0x8b, 0xb3, 0xb3, 0x0e, 0xb7, 0x5d,
                                 0x9b, 0xc4, 0x0e, 0xe8, 0x2b, 0xf2, 0x01, 0x24, 0xec, 0x20, 0x4f, 0xc3, 0x26, 0x51,
                                 0xd4, 0x0b, 0xbb, 0x09, 0xfa, 0x6f, 0x20, 0x50, 0x55, 0x77, 0x21, 0x90, 0xf0, 0x2b,
                                 0x41, 0x78, 0x4a, 0xef, 0x08, 0x8a, 0xc5, 0x57, 0x4d, 0xc4, 0x72, 0x0c, 0x02, 0x09,
                                 0x9f, 0x21, 0xb8, 0x3d, 0xd2, 0x30, 0x8f, 0x2b, 0x2b, 0xc2, 0x0f, 0xb4, 0xa9, 0xb5,
                                 0xf4, 0xc0, 0x13, 0xf8, 0xf4, 0xc1, 0xd2, 0x56, 0xc3, 0x10, 0x65, 0xe7, 0x89, 0x1e,
                                 0x58, 0x11, 0x69, 0xa3, 0x80, 0xaa, 0xb0, 0x2c, 0x40, 0xa4, 0x73, 0xd1, 0x50, 0x88,
                                 0xc5, 0x52, 0xf3, 0x0e, 0x8d, 0x3a, 0xe8, 0x82, 0x1b, 0x7a, 0x18, 0xd3, 0xca, 0x25,
                                 0x13, 0x7b, 0xfc, 0xc0, 0xe2, 0xac, 0xa3, 0x1d, 0x6b, 0x03, 0x39, 0x51, 0x34, 0x44,
                                 0xdc, 0xba, 0xaa, 0x94, 0xb8, 0xab, 0x6a, 0x2f, 0x6f, 0x5c, 0xfe, 0xe8, 0x5a, 0xbe,
                                 0xb7, 0x96, 0x3b, 0x8b, 0xfe, 0xec, 0xb5, 0xb3, 0xe8, 0x53, 0x46, 0x9b, 0x3c, 0x36,
                                 0xce, 0x7f, 0xa2, 0xdf, 0x83, 0x64, 0xf6, 0x56, 0x75, 0x32, 0xea, 0xae, 0xd2, 0xec,
                                 0x64, 0xa1, 0xa5, 0xe5, 0x5f, 0xfe, 0x77, 0x75, 0x79, 0x63, 0xc3, 0xa0, 0xb3, 0x91,
                                 0x3e, 0x33, 0xe1, 0xd4, 0x52, 0x61, 0xbb, 0x45, 0xb4, 0xfc, 0xdf, 0x7d, 0xb5, 0x7b,
                                 0xc9, 0xb4, 0xf9, 0x9f, 0xf7, 0x05, 0xda, 0xb4, 0x87, 0xb4, 0xef, 0x0d, 0x00, 0x00};

    // required header for gzip encoding
    server.sendHeader("Content-Encoding", "gzip");
    server.send(200, "text/html", (const char *) setupHtml, SETUP_HTML_LENGTH);
}

// web page on wrong address
void handleNotFound() {
    server.send(404, "text/plain", "404: Not found");
}

// web page on successful data entry
void handleSuccess() {
    server.send(200, "text/plain", "Data entry successful!");
    inputReceivedTime = millis();
}

void handleInput() {
    server.arg("SSID").toCharArray(inputSSID, 32);
    server.arg("Password").toCharArray(inputPass, 65);

    PRINTLN(inputSSID);
    PRINTLN(inputPass);

    inputMqtt[0] = server.arg("MQTT0").toInt();
    inputMqtt[1] = server.arg("MQTT1").toInt();
    inputMqtt[2] = server.arg("MQTT2").toInt();
    inputMqtt[3] = server.arg("MQTT3").toInt();

    PRINTLN(inputMqtt[0]);
    PRINTLN(inputMqtt[1]);
    PRINTLN(inputMqtt[2]);
    PRINTLN(inputMqtt[3]);

    server.arg("DeviceName").toCharArray(inputDeviceName, 21);

    PRINTLN(inputDeviceName);

    // generate status topic (required for connecting to broker with LWT)
    strcpy(topicStatus, "HADIS/");
    strcat(topicStatus, inputDeviceName);
    strcat(topicStatus, "/STATUS");

    server.sendHeader("Location", String("/success"), true);
    server.send(302, "text/plain", "Data entry successful!");
}


// ------------------------------------------------------------------
// HOST AP    HOST AP    HOST AP    HOST AP    HOST AP    HOST AP
// ------------------------------------------------------------------

// host access point for settings entry
void hostAP() {

    // reset long press flag to re-enable single press
    longPressB1Finished = FALSE;
    longPressB2Finished = FALSE;

    // LED is on during Access point hosting
    digitalWrite(LED_indicator, LOW);

    // send offline status when hosting
    pubSubClient.publish(topicStatus, "OFFLINE", TRUE);

    setupServer();

    unsigned long startTime = millis();
    unsigned long currentTime;

    // run server until timeout (AP_HOST_TIMEOUT)
    while (TRUE) {
        server.handleClient();

        currentTime = millis();
        if (currentTime - startTime > AP_HOST_TIMEOUT) break;

        // poll for short press
        shortPressPoll();

        // stop server after AP_INPUT_RECEIVED_DELAY ms of receiving input
        if (inputReceivedTime > 1 && currentTime - inputReceivedTime > AP_INPUT_RECEIVED_DELAY) break;
    }

    PRINTLN("AP closing");

    //reset long press counter if button clicked while hosting AP
    longPressB1 = FALSE;
    longPressB2 = FALSE;

    // shut down AP mode
    server.close();
    WiFi.softAPdisconnect(true);

    // configure back to wifi mode & configure MQTT server with new input settings
    WiFi.mode(WIFI_STA);
    WiFi.hostname("HADIS-Duo");

    // if input was received try new connection parameters
    if (inputReceivedTime > 1) {
        WiFi.begin(inputSSID, inputPass);
        pubSubClient.setServer(inputMqtt, 1883);
    }
        // use old parameters
    else {
        WiFi.begin(deviceSettings.ssid, deviceSettings.password);
        pubSubClient.setServer(deviceSettings.mqtt, 1883);
    }
    delay(500);

    // turn off LED
    digitalWrite(LED_indicator, HIGH);
}


// ------------------------------------------------------------------
// SETUP SERVER   SETUP SERVER    SETUP SERVER    SETUP SERVER
// ------------------------------------------------------------------

// configures the access point
void setupServer() {
    PRINTLN("Configuring access point...");

    // disconnect from wifi & set adapter mode to Access Point
    pubSubClient.disconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);

    // configure AP
    IPAddress local_IP(10, 0, 0, 1);
    IPAddress gateway(10, 0, 0, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    delay(1000);

    // build AP SSID from device's mac address
    strcpy(ssidAP, "HADIS-AP: ");
    strcat(ssidAP, macAddress);
    PRINTLN(ssidAP);

    // start the AP
    WiFi.softAP(ssidAP);

    PRINT("AP IP address: ");
    PRINTLN(WiFi.softAPIP());

    // server page function definitions
    server.on("/", handleRoot);
    server.on("/input", HTTP_POST, handleInput);
    server.on("/success", handleSuccess);
    server.onNotFound(handleNotFound);
    server.begin();

    PRINTLN("HTTP server started");
}

// build string of devices mac address
// code copied from <ESP8266WiFi> to remove usage of STRING
void generateMacAddress() {
    uint8_t mac[6];
    wifi_get_macaddr(STATION_IF, mac);

    sprintf(macAddress, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    PRINT("MAC: ");
    PRINTLN(macAddress);
}


// ------------------------------------------------------------------
// EEPROM    EEPROM    EEPROM    EEPROM    EEPROM    EEPROM    EEPROM
// ------------------------------------------------------------------

// store struct with device setting (EepromStorageStruct) to EEPROM
void writeEeprom() {
    PRINTLN("Writing EEPROM");

    EEPROM.begin(512);
    EEPROM.put(0, deviceSettings);
    EEPROM.commit();
    EEPROM.end();

    delay(1000);
}

// Store new input from Access Point if provided
void shouldWriteEEPROM() {
    // if client successfully connected, write new device settings to EEPROM
    if (inputReceivedTime > 0) {

        // copy AP input to deviceSettings struct
        strcpy(deviceSettings.ssid, inputSSID);
        strcpy(deviceSettings.password, inputPass);
        strcpy(deviceSettings.deviceName, inputDeviceName);

        for (int i = 0; i < 4; i++)
            deviceSettings.mqtt[i] = inputMqtt[i];

        // write the struct to EEPROM
        writeEeprom();

        // generate new MQTT topic strings
        generateTopics();

        // reset input time
        inputReceivedTime = RESET;
    }
}
// ------------------------------------------------------------------
// SHORT PRESS    SHORT PRESS    SHORT PRESS    SHORT PRESS
// ------------------------------------------------------------------

// function for short press of device button 1
void B1shortPress() {
    PRINTLN("short press B1");

    // toggle relay1 state
    relay1State = !relay1State;
    digitalWrite(RELAY1, relay1State);

#ifdef DEBUG
    PRINT("Relay1 state: ");
    PRINTLN(relay1State);
#endif

    // boolean for preventing feedback loop on topic
    hardwareTrigger1 = TRUE;

    // convert RELAY1 state to char array & publish
    itoa(relay1State, msg, 10);
    pubSubClient.publish(topicSwitch1, msg, TRUE);
}

// function for short press of device button 2
void B2shortPress() {
    PRINTLN("short press B2");

    // toggle relay2 state
    relay2State = !relay2State;
    digitalWrite(RELAY2, relay2State);

#ifdef DEBUG
    PRINT("Relay2 state: ");
    PRINTLN(relay2State);
#endif

    // boolean for preventing feedback loop on topic
    hardwareTrigger2 = TRUE;

    // convert RELAY2 state to char array & publish
    itoa(relay2State, msg, 10);
    pubSubClient.publish(topicSwitch2, msg, TRUE);
}

// short press polling code
// code runs every main loop cycle
void shortPressPoll() {

    if(shortPressB1) {
        shortPressB1 = FALSE;
        B1shortPress();
    }

    if(shortPressB2){
        shortPressB2 = FALSE;
        B2shortPress();
    }


}

// ------------------------------------------------------------------
// LONG PRESS    LONG PRESS    LONG PRESS    LONG PRESS    LONG PRESS
// ------------------------------------------------------------------

// function for long press of device button 1
void B1longPress() {
    PRINTLN("long press B1");

    // switch device to Access Point mode &
    // turn built-in LED ON during this time
    hostAP();
}

// function for long press of device button 2
void B2longPress() {
    PRINTLN("long press B2");

    // send message about long pressing secondary button
    pubSubClient.publish(topicLong2, "1", FALSE);
}

// long press polling code
// code runs every main loop cycle
void longPressPoll() {

    // button1
    if(longPressB1){
        longPressB1Finished = TRUE;
        longPressB1 = FALSE;
        B1longPress();
    }

    // reset long press flag for single function activation
    if(longPressB1Finished && digitalRead(pinButton1)) {
        longPressB1Finished = FALSE;
    }

    // button2
    if(longPressB2) {
        longPressB2Finished = TRUE;
        longPressB2 = FALSE;
        B2longPress();
    }

    // reset long press flag for single function activation
    if(longPressB2Finished && digitalRead(pinButton2)) {
        longPressB2Finished = FALSE;
    }
}


// ------------------------------------------------------------------
// GENERATE TOPIC    GENERATE TOPIC    GENERATE TOPIC
// ------------------------------------------------------------------

// generate MQTT topic strings
// HADIS/*deviceName*/SWITCH1 -> handle relay1 state
// HADIS/*deviceName*/SWITCH2 -> handle relay2 state
// HADIS/*deviceName*/LONG2 -> send message when long press button2
// HADIS/*deviceName*/SETUP -> handle setup mode activation
// HADIS/*deviceName*/STATUS -> handle device status (ONLINE/OFFLINE)
// *deviceName* is a placeholder for specific topics
void generateTopics() {

    // build topic prefix
    strcpy(topicSetup, "HADIS/");
    strcat(topicSetup, deviceSettings.deviceName);

    // build topic suffix
    strcpy(topicSwitch1, topicSetup);
    strcpy(topicSwitch2, topicSetup);
    strcpy(topicLong2, topicSetup);
    strcpy(topicStatus, topicSetup);
    strcat(topicSetup, "/SETUP");
    strcat(topicSwitch1, "/SWITCH1");
    strcat(topicSwitch2, "/SWITCH2");
    strcat(topicLong2, "/LONG2");
    strcat(topicStatus, "/STATUS");

    PRINTLN(topicSwitch1);
    PRINTLN(topicSwitch2);
    PRINTLN(topicLong2);
    PRINTLN(topicSetup);
    PRINTLN(topicStatus);
}


// ------------------------------------------------------------------
// RECONNECT    RECONNECT    RECONNECT    RECONNECT    RECONNECT
// ------------------------------------------------------------------
void connectToMqtt() {

    // Build client id from MAC address
    char clientID[31];
    strcpy(clientID, "HADIS-Client-");
    strcat(clientID, macAddress);
    PRINTLN(clientID);

    PRINT("Trying to connect to MQTT server...");

    // Attempt to connect
    if (pubSubClient.connect(clientID, topicStatus, 0, TRUE, "OFFLINE")) {
        PRINTLN("connected");

        // check if connection was established with new settings from AP
        shouldWriteEEPROM();

        // publish status = connected to broker
        pubSubClient.publish(topicStatus, "ONLINE", TRUE);

        // on successful connection publish current RELAY1 state
        itoa(relay1State, msg, 10);
        pubSubClient.publish(topicSwitch1, msg, TRUE);

        // on successful connection publish current RELAY2 state
        itoa(relay2State, msg, 10);
        pubSubClient.publish(topicSwitch2, msg, TRUE);

        // subscribe to topics
        pubSubClient.subscribe(topicSwitch1);
        pubSubClient.subscribe(topicSwitch2);
        pubSubClient.subscribe(topicSetup);

    } else {

        // if connection can't be established set flag on inputReceivedTime that data was still received,
        // but it won't interrupt with Access point hosting
        if (inputReceivedTime > 1)
            inputReceivedTime = 1;

#ifdef DEBUG
        PRINT("FAILED, reason: ");

        // reason for unsuccessful connection
        switch (pubSubClient.state()) {
            case -4:
                PRINT("MQTT_CONNECTION_TIMEOUT");
                break;
            case -3:
                PRINT("MQTT_CONNECTION_LOST");
                break;
            case -2:
                PRINT("MQTT_CONNECT_FAILED");
                break;
            case -1:
                PRINT("MQTT_DISCONNECTED");
                break;

            case 1:
                PRINT("MQTT_CONNECT_BAD_PROTOCOL");
                break;

            case 2:
                PRINT("MQTT_CONNECT_BAD_CLIENT_ID");
                break;

            case 3:
                PRINT("MQTT_CONNECT_UNAVAILABLE");
                break;

            case 4:
                PRINT("MQTT_CONNECT_BAD_CREDENTIALS");
                break;

            case 5:
                PRINT("MQTT_CONNECT_UNAUTHORIZED");
                break;
        }

        PRINT(" (");
        PRINT(pubSubClient.state());
        PRINTLN(")");
        PRINTLN("Try again in 10 seconds");
#endif
    }
}


// ------------------------------------------------------------------
// CALLBACK    CALLBACK    CALLBACK    CALLBACK    CALLBACK
// ------------------------------------------------------------------
void callback(char *topic, const byte *payload, unsigned int length) {
    PRINT("Message arrived [");
    PRINT(topic);
    PRINTLN("] ");

    // topic for RELAY1 control
    if (!strcmp(topic, topicSwitch1)) {

        // only change RELAY1 state if command not issued by this device
        if (!hardwareTrigger1) {

            // set received relay state
            relay1State = payload[0] - '0';
            digitalWrite(RELAY1, relay1State);

        } else {
            hardwareTrigger1 = FALSE;
        }
    }
        // topic for RELAY2 control
    else if (!strcmp(topic, topicSwitch2)) {

        // only change RELAY2 state if command not issued by this device
        if (!hardwareTrigger2) {

            // set received relay state
            relay2State = payload[0] - '0';
            digitalWrite(RELAY2, relay2State);

        } else {
            hardwareTrigger2 = FALSE;
        }
    }
        // topic for starting setup mode
    else if (!strcmp(topic, topicSetup) && payload[0] == '1') {
        PRINTLN("SETUP REQUESTED");
        hostAP();
    }
}


// ------------------------------------------------------------------
// SETUP    SETUP    SETUP    SETUP    SETUP    SETUP    SETUP
// ------------------------------------------------------------------

// setup HADIS device on startup
void setup() {
    //Read deviceSettings form eeprom address 0
    EEPROM.begin(512);
    EEPROM.get(0, deviceSettings);
    EEPROM.end();

    delay(1000);
    SERIAL_BEGIN(9600);

    // setup pin modes
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(LED_indicator, OUTPUT);
    pinMode(pinButton1, INPUT_PULLUP);
    pinMode(pinButton2, INPUT_PULLUP);

    // turn off indicator led
    digitalWrite(LED_indicator, HIGH);

    // setup button press interrupt timer
    /* Dividers:
		TIM_DIV1 = 0,   //80MHz (80 ticks/us - 104857.588 us max)
		TIM_DIV16 = 1,  //5MHz (5 ticks/us - 1677721.4 us max)
		TIM_DIV256 = 3  //312.5Khz (1 tick = 3.2us - 26843542.4 us max)
	Reloads:
		TIM_SINGLE	0 //on interrupt routine you need to write a new value to start the timer again
		TIM_LOOP	1 //on interrupt the counter will start with the same value again
	*/
    timer1_attachInterrupt(buttonISR);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    // Arm the Timer for 0.01s interval
    // 5MHz * 50000 = 100Hz
    timer1_write(50000);

    // connect to wifi & setup MQTT server data
    WiFi.mode(WIFI_STA);
    WiFi.hostname("HADIS-Duo");
    WiFi.begin(deviceSettings.ssid, deviceSettings.password);
    pubSubClient.setServer(deviceSettings.mqtt, 1883);
    pubSubClient.setCallback(callback);

    // build device specific MQTT topics
    generateTopics();

    // build device's mac address string
    generateMacAddress();

    delay(1000);
}


// ------------------------------------------------------------------
// MAIN LOOP    MAIN LOOP    MAIN LOOP    MAIN LOOP    MAIN LOOP
// ------------------------------------------------------------------
unsigned int previousReconnectTryTime = 0;

// main device operation loop
void loop() {

    // check for connection to MQTT broker & try to reconnect every CONNECTION_RETRY_DELAY ms
    if (!pubSubClient.connected()) {
        if (millis() - previousReconnectTryTime >= CONNECTION_RETRY_DELAY) {
            connectToMqtt();
            previousReconnectTryTime = millis();
        }
    } else {
        pubSubClient.loop();
    }

    // poll for short press
    shortPressPoll();

    // poll for long press
    longPressPoll();
}
