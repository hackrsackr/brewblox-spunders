#include "secrets.h"

// Number of sensor solenoid pairs
#define _NUMBER_OF_SPUNDERS 4

// Relay logic
#define _RELAY_OPEN HIGH

// Reference voltage
#define _VESP 4.47 // actual voltage of esp 5v channel
#define _PUSB 5.00 // actual voltage the pi usb supplies

// WiFi credentials
#define _SSID SECRET_SSID
#define _PASS SECRET_PASS

// MQTT setup
#define _MQTTHOST "192.168.1.2"
#define _MQTTPORT 1883
#define _CLIENTID "spunders"
#define _SUBTOPIC "brewcast/history/spark-two"
#define _PUBTOPIC "brewcast/history/spunders"

// Name of each spunder object
#define _NAME1 "spunder1"
#define _NAME2 "spunder2"
#define _NAME3 "spunder3"
#define _NAME4 "spunder4"

std::array<const String, _NUMBER_OF_SPUNDERS>
    SPUNDER_NAMES = {_NAME1, _NAME2, _NAME3, _NAME4};

// Brewblox names of temps to request for each spunder mqtt_temp_field
String _TEMP1 = "TEMP_ambientRoom";
String _TEMP2 = "TEMP_ambientRoom";
String _TEMP3 = "TEMP_blueBeer";
String _TEMP4 = "TEMP_orangeBeer";

std::array<const String, _NUMBER_OF_SPUNDERS>
    MQTT_FIELDS = {_TEMP1, _TEMP2, _TEMP3, _TEMP4};

// Desired vols of CO2 for each spunder
float _VOLS1 = 2.5;
float _VOLS2 = 2.5;
float _VOLS3 = 2.5;
float _VOLS4 = 2.5;

std::array<float, _NUMBER_OF_SPUNDERS>
    DESIRED_VOLS = {_VOLS1, _VOLS2, _VOLS3, _VOLS4};

// Max units of each pressure sensor in psi
#define _UMAX1 30.0
#define _UMAX2 60.0
#define _UMAX3 60.0
#define _UMAX4 60.0

std::array<const float, _NUMBER_OF_SPUNDERS>
    UNIT_MAXS = {_UMAX1, _UMAX2, _UMAX3, _UMAX4};

// Pin of the relay of each spunder
#define _RPIN1 14
#define _RPIN2 27
#define _RPIN3 16
#define _RPIN4 17

std::array<const uint8_t, _NUMBER_OF_SPUNDERS>
    RELAY_PINS = {_RPIN1, _RPIN2, _RPIN3, _RPIN4};

// Voltage at zero pressure using esp vcc
#define _ESPV1 .54
#define _ESPV2 .43
#define _ESPV3 .43
#define _ESPV4 .42

std::array<const float, _NUMBER_OF_SPUNDERS>
    ESP_VOLTS = {_ESPV1, _ESPV2, _ESPV3, _ESPV4};

// Voltage at zero pressure using pi usb as 5v source
#define _PUSB1 .54
#define _PUSB2 .48
#define _PUSB3 .47
#define _PUSB4 .46

std::array<const float, _NUMBER_OF_SPUNDERS>
    USB_VOLTS = {_PUSB1, _PUSB2, _PUSB3, _PUSB4};

// Server setpoint message fields
String setpointMessage1 = String(DESIRED_VOLS[0]);
String setpointMessage2 = String(DESIRED_VOLS[1]);
String setpointMessage3 = String(DESIRED_VOLS[2]);
String setpointMessage4 = String(DESIRED_VOLS[3]);

// Server mqtt temp message fields
String mqttMessage1 = MQTT_FIELDS[0];
String mqttMessage2 = MQTT_FIELDS[1];
String mqttMessage3 = MQTT_FIELDS[2];
String mqttMessage4 = MQTT_FIELDS[3];

// Server setpoint inputs
#define _SPIN1 "setpoint_input1"
#define _SPIN2 "setpoint_input2"
#define _SPIN3 "setpoint_input3"
#define _SPIN4 "setpoint_input4"

std::array<const char *, _NUMBER_OF_SPUNDERS>
    _SETPOINT_INPUTS = {_SPIN1, _SPIN2, _SPIN3, _SPIN4};

// Server mqtt temp field inputs
#define _MQIN1 "mqtt_input1"
#define _MQIN2 "mqtt_input2"
#define _MQIN3 "mqtt_input3"
#define _MQIN4 "mqtt_input4"

std::array<const char *, _NUMBER_OF_SPUNDERS>
    _MQTT_INPUTS = {_MQIN1, _MQIN2, _MQIN3, _MQIN4};
