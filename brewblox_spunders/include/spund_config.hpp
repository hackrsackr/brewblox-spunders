#include "secrets.h"

// WiFi credentials
#define _SSID SECRET_SSID
#define _PASS SECRET_PASS

// Brewblox IP address
#define _MQTTHOST "192.168.1.2"
#define _MQTTPORT 1883
#define _CLIENTID "spunders"
#define _SUBTOPIC "brewcast/history/spark-two"
#define _PUBTOPIC "brewcast/history/spunders"

// Brewblox names of temps to request for each spunder mqtt_temp_field
#define _TEMP1 "TEMP_orangeBeer"
#define _TEMP2 "TEMP_orangeFridge"
#define _TEMP3 "TEMP_blueBeer"
#define _TEMP4 "TEMP_blueFridge"

// Number of spunders and relay logic
#define _NUMBER_OF_SPUNDERS 4
#define _RELAY_OPEN HIGH

// Name of each spunder object
#define _NAME1 "spunder1"
#define _NAME2 "spunder2"
#define _NAME3 "spunder3"
#define _NAME4 "spunder4"

// Desired vols of CO2 for each spunder
#define _VOLS1 2.5
#define _VOLS2 2.5
#define _VOLS3 3.0
#define _VOLS4 4.0

// Max units of each pressure sensor in psi
#define _UMAX1 60
#define _UMAX2 60
#define _UMAX3 60
#define _UMAX4 60

// Pin of the relay of each spunder
#define _RPIN1 IO14
#define _RPIN2 IO27
#define _RPIN3 IO16
#define _RPIN4 IO17

// Zero psi volatage reading
#define _OFFS1 .42
#define _OFFS2 .42
#define _OFFS3 .44
#define _OFFS4 .44

const int UNIT_MAXS[_NUMBER_OF_SPUNDERS]      = { _UMAX1, _UMAX2, _UMAX3, _UMAX4 };
const int RELAY_PINS[_NUMBER_OF_SPUNDERS]     = { _RPIN1, _RPIN2, _RPIN3, _RPIN4 };
const float DESIRED_VOLS[_NUMBER_OF_SPUNDERS] = { _VOLS1, _VOLS2, _VOLS3, _VOLS4 };
const float OFFSETS[_NUMBER_OF_SPUNDERS]      = { _OFFS1, _OFFS2, _OFFS3, _OFFS4 };
String SPUNDER_NAMES[_NUMBER_OF_SPUNDERS]     = { _NAME1, _NAME2, _NAME3, _NAME4 };
String MQTT_TEMP_FIELDS[_NUMBER_OF_SPUNDERS]  = { _TEMP1, _TEMP2, _TEMP3, _TEMP4 };