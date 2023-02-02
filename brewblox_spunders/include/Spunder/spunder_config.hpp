#include "secrets.h"

// Number of sensor solenoid pairs
const int _NUMBER_OF_SPUNDERS = 4;

// Relay logic
const bool _RELAY_OPEN = HIGH;

// Reference voltage
const float _VESP = 4.47; // actual voltage of esp 5v channel
const float _PUSB = 5.00; // actual voltage the pi usb supplies

// WiFi credentials
const char *_SSID = SECRET_SSID;
const char *_PASS = SECRET_PASS;

// MQTT setup
const char *_MQTTHOST = "192.168.1.2";
const short _MQTTPORT = 1883;
const char *_CLIENTID = "spunders";
const char *_SUBTOPIC = "brewcast/history/spark-two";
const char *_PUBTOPIC = "brewcast/history/spunders";

// Name of each spunder object
const String _NAME1 = "spunder1";
const String _NAME2 = "spunder2";
const String _NAME3 = "spunder3";
const String _NAME4 = "spunder4";

std::array<const String, _NUMBER_OF_SPUNDERS>
    _SPUNDER_NAMES = {_NAME1, _NAME2, _NAME3, _NAME4};

// Brewblox names of temps to request for each spunder mqtt_temp_field
String _TEMP1 = "TEMP_ambientRoom";
String _TEMP2 = "TEMP_ambientRoom";
String _TEMP3 = "TEMP_ambientRoom";
String _TEMP4 = "TEMP_ambientRoom";

std::array<String, _NUMBER_OF_SPUNDERS>
    _MQTT_FIELDS = {_TEMP1, _TEMP2, _TEMP3, _TEMP4};

// Desired vols of CO2 for each spunder
float _VOLS1 = 2.2;
float _VOLS2 = 2.2;
float _VOLS3 = 2.2;
float _VOLS4 = 2.2;

std::array<float, _NUMBER_OF_SPUNDERS>
    _DESIRED_VOLS = {_VOLS1, _VOLS2, _VOLS3, _VOLS4};

// Max units of each pressure sensor in psi
const uint8_t _UMAX1 = 30;
const uint8_t _UMAX2 = 60;
const uint8_t _UMAX3 = 60;
const uint8_t _UMAX4 = 60;

std::array<const uint8_t, _NUMBER_OF_SPUNDERS>
    _UNIT_MAXS = {_UMAX1, _UMAX2, _UMAX3, _UMAX4};

// Pin of the relay of each spunder
const uint8_t _RPIN1 = 14;
const uint8_t _RPIN2 = 27;
const uint8_t _RPIN3 = 16;
const uint8_t _RPIN4 = 17;

std::array<const uint8_t, _NUMBER_OF_SPUNDERS>
    _RELAY_PINS = {_RPIN1, _RPIN2, _RPIN3, _RPIN4};

// Voltage at zero pressure using esp vcc
const float _ESPV1 = .54;
const float _ESPV2 = .43;
const float _ESPV3 = .43;
const float _ESPV4 = .42;

std::array<const float, _NUMBER_OF_SPUNDERS>
    _ESP_VOLTS = {_ESPV1, _ESPV2, _ESPV3, _ESPV4};

// Voltage at zero pressure using pi usb as 5v source
const float _PUSB1 = .54;
const float _PUSB2 = .48;
const float _PUSB3 = .47;
const float _PUSB4 = .46;

std::array<const float, _NUMBER_OF_SPUNDERS>
    _USB_VOLTS = {_PUSB1, _PUSB2, _PUSB3, _PUSB4};

// Server setpoint message fields
String _SPMS1 = String(_DESIRED_VOLS[0]);
String _SPMS2 = String(_DESIRED_VOLS[1]);
String _SPMS3 = String(_DESIRED_VOLS[2]);
String _SPMS4 = String(_DESIRED_VOLS[3]);

std::array<String, _NUMBER_OF_SPUNDERS>
    _SETPOINT_MESSAGES = {_SPMS1, _SPMS2, _SPMS3, _SPMS4};

// Server mqtt temp message fields
String _MQMS1 = _MQTT_FIELDS[0];
String _MQMS2 = _MQTT_FIELDS[1];
String _MQMS3 = _MQTT_FIELDS[2];
String _MQMS4 = _MQTT_FIELDS[3];

std::array<String, _NUMBER_OF_SPUNDERS>
    _MQTT_MESSAGES = {_MQMS1, _MQMS2, _MQMS3, _MQMS4};

// Server setpoint inputs
const String _SPIN1 = "setpoint_input1";
const String _SPIN2 = "setpoint_input2";
const String _SPIN3 = "setpoint_input3";
const String _SPIN4 = "setpoint_input4";

std::array<String, _NUMBER_OF_SPUNDERS>
    _SETPOINT_INPUTS = {_SPIN1, _SPIN2, _SPIN3, _SPIN4};

// Server mqtt temp field inputs
const String _MQIN1 = "mqtt_input1";
const String _MQIN2 = "mqtt_input2";
const String _MQIN3 = "mqtt_input3";
const String _MQIN4 = "mqtt_input4";

std::array<String, _NUMBER_OF_SPUNDERS>
    _MQTT_INPUTS = {_MQIN1, _MQIN2, _MQIN3, _MQIN4};
