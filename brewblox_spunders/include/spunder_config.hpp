#include "secrets.h"

// Number of sensor solenoid pairs
#define _NUMBER_OF_SPUNDERS 4

// Relay logic
#define _RELAY_OPEN HIGH

// Esp USB actual voltage
#define _VUSB 4.42

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

// Brewblox names of temps to request for each spunder mqtt_temp_field
#define _TEMP1 "TEMP_orangeBeer"
#define _TEMP2 "TEMP_orangeFridge"
#define _TEMP3 "TEMP_blueBeer"
#define _TEMP4 "TEMP_blueFridge"

// Desired vols of CO2 for each spunder
#define _VOLS1 2.5
#define _VOLS2 2.5
#define _VOLS3 3.0
#define _VOLS4 4.0

// Max units of each pressure sensor in psi
#define _UMAX1 30.0
#define _UMAX2 60.0
#define _UMAX3 60.0
#define _UMAX4 60.0

// Pin of the relay of each spunder
#define _RPIN1 14
#define _RPIN2 27
#define _RPIN3 16
#define _RPIN4 17

// Voltage at zero pressure
#define _OFFS1 .53
#define _OFFS2 .44
#define _OFFS3 .44
#define _OFFS4 .44

std::array<const String,  _NUMBER_OF_SPUNDERS> SPUNDER_NAMES = { _NAME1, _NAME2, _NAME3, _NAME4 };
std::array<const String,  _NUMBER_OF_SPUNDERS> MQTT_FIELDS   = { _TEMP1, _TEMP2, _TEMP3, _TEMP4 };
std::array<const float,   _NUMBER_OF_SPUNDERS> DESIRED_VOLS  = { _VOLS1, _VOLS2, _VOLS3, _VOLS4 };
std::array<const float,   _NUMBER_OF_SPUNDERS> UNIT_MAXS     = { _UMAX1, _UMAX2, _UMAX3, _UMAX4 };
std::array<const uint8_t, _NUMBER_OF_SPUNDERS> RELAY_PINS    = { _RPIN1, _RPIN2, _RPIN3, _RPIN4 };
std::array<const float,   _NUMBER_OF_SPUNDERS> OFFSET_VOLTS  = { _OFFS1, _OFFS2, _OFFS3, _OFFS4 };
