// Brewblox IP address
#define _MQTTHOST "192.168.1.2"
#define _MQTTPORT 1883
#define _CLIENTID "spunders"
#define _SUBTOPIC "brewcast/history/spark-two"
#define _PUBTOPIC "brewcast/history/spunders"

// Max units of each pressure sensor in psi.
#define _UMAX1 60
#define _UMAX2 60
#define _UMAX3 60
#define _UMAX4 60

// Brewblox names of temps to request for each spunder
#define _TEMP1 "TEMP_orangeBeer"
#define _TEMP2 "TEMP_orangeFridge"
#define _TEMP3 "TEMP_blueBeer"
#define _TEMP4 "TEMP_blueFridge"

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

// Pin of the relay of each spunder
#define _RPIN1 IO14
#define _RPIN2 IO27
#define _RPIN3 IO16
#define _RPIN4 IO17

// Zero psi volatage reading
#define _OFFS1 .44
#define _OFFS2 .44
#define _OFFS3 .46
#define _OFFS4 .46
