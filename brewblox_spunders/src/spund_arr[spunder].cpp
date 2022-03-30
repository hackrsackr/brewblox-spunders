#include <Arduino.h>
#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include <Esp32HTTPUpdateServer.h>
#include <math.h>

#include "secrets.hpp"
#include "spund_config.hpp"
#include "spunder.hpp"

#define NUMBER_OF_SPUNDERS 4
#define RELAY_OPEN HIGH

// Create array of Spunders
Spunder spund_arr[NUMBER_OF_SPUNDERS];

JSONVar parsed_data; 

// From spund_config.h
const int UNIT_MAXS[NUMBER_OF_SPUNDERS]      = { _UMAX1, _UMAX2, _UMAX3, _UMAX4 };
const int RELAY_PINS[NUMBER_OF_SPUNDERS]     = { _RPIN1, _RPIN2, _RPIN3, _RPIN4 };
float DESIRED_VOLS[NUMBER_OF_SPUNDERS]       = { _VOLS1, _VOLS2, _VOLS3, _VOLS4 };
float OFFSETS[NUMBER_OF_SPUNDERS]            = { _OFFS1, _OFFS2, _OFFS3, _OFFS4 };
String SPUNDER_NAMES[NUMBER_OF_SPUNDERS]     = { _NAME1, _NAME2, _NAME3, _NAME4 };
String MQTT_TEMP_FIELDS[NUMBER_OF_SPUNDERS]  = { _TEMP1, _TEMP2, _TEMP3, _TEMP4 };

// From spund_config.h
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_ONE);
  client.enableHTTPWebUpdater();
  client.setMaxPacketSize(4096);
  //client.enableDebuggingMessages();

  WiFi.begin(_SSID, _PASS);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  for (int spunder = 0; spunder < NUMBER_OF_SPUNDERS; spunder++)
  {
    spund_arr[spunder].name          = SPUNDER_NAMES[spunder];
    spund_arr[spunder].mqtt_field    = MQTT_TEMP_FIELDS[spunder];
    spund_arr[spunder].relay_pin     = RELAY_PINS[spunder];
    spund_arr[spunder].unit_max      = UNIT_MAXS[spunder];
    spund_arr[spunder].vols_setpoint = DESIRED_VOLS[spunder];
    spund_arr[spunder].zero_volts    = OFFSETS[spunder];
    spund_arr[spunder].stored_time   = millis();
    spund_arr[spunder].ads_channel   = spunder;
    spund_arr[spunder].tempC         = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();

    pinMode(spund_arr[spunder].relay_pin, OUTPUT);
    digitalWrite(spund_arr[spunder].relay_pin, !RELAY_OPEN);
  }
}

void onConnectionEstablished()
{
  client.subscribe(_SUBTOPIC, [](const String &payload)
  {
    JSONVar data, message; 

    // Get the JSON data of the sub_topic
    parsed_data = JSON.parse(payload);
    if (JSON.typeof(parsed_data) == "undefined") 
    {
      Serial.println("Parsing input failed!");
      return;
    }

    // Read each spunder in the array
    for (int spunder = 0; spunder < NUMBER_OF_SPUNDERS; spunder++)
    {
      // Parse message from _MQTTHOST to get temperature value needed 
      float raw_temp = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();
      
      //Serial.println(raw_temp);
      // Filter outliers
      if (spund_arr[spunder].tempC - raw_temp < .5)
      {spund_arr[spunder].tempC = raw_temp; }
      
      // Get data values, test carb, vent if neccessary
      spund_arr[spunder].tempF           = spund_arr[spunder].convert_temp();
      spund_arr[spunder].psi_value       = spund_arr[spunder].get_psi_value();
      spund_arr[spunder].psi_setpoint    = spund_arr[spunder].get_psi_setpoint();
      spund_arr[spunder].vols_value      = spund_arr[spunder].get_vols();
      spund_arr[spunder].time_since_vent = spund_arr[spunder].test_carb();

      // Populate data message    
      data[spund_arr[spunder].name]["volts"]        = spund_arr[spunder].volts;
      data[spund_arr[spunder].name]["tempC"]        = spund_arr[spunder].tempC;
      data[spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
      data[spund_arr[spunder].name]["psi"]          = spund_arr[spunder].psi_value;
      data[spund_arr[spunder].name]["vols_target"]  = spund_arr[spunder].vols_setpoint;
      data[spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
      data[spund_arr[spunder].name]["since_vent"]   = spund_arr[spunder].time_since_vent;
    }

    Serial.println(JSON.stringify(data));
    Serial.println("");

    message["key"] = "spunders";
    message["data"] = data;

    client.publish(_PUBTOPIC, JSON.stringify(message));
  });

  delay(5000);
}

void loop()
{
  client.loop();
}