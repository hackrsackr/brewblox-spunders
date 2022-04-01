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

// From spund_config.h
const int UNIT_MAXS[NUMBER_OF_SPUNDERS]      = { _UMAX1, _UMAX2, _UMAX3, _UMAX4 };
const int RELAY_PINS[NUMBER_OF_SPUNDERS]     = { _RPIN1, _RPIN2, _RPIN3, _RPIN4 };
const float DESIRED_VOLS[NUMBER_OF_SPUNDERS] = { _VOLS1, _VOLS2, _VOLS3, _VOLS4 };
String SPUNDER_NAMES[NUMBER_OF_SPUNDERS]     = { _NAME1, _NAME2, _NAME3, _NAME4 };
String MQTT_TEMP_FIELDS[NUMBER_OF_SPUNDERS]  = { _TEMP1, _TEMP2, _TEMP3, _TEMP4 };

// Connect to brewblox mqtt host
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

// Initialize an array of spunder objects
Spunder spunder_arr[NUMBER_OF_SPUNDERS];

JSONVar parsed_data;

void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_ONE);
  client.enableHTTPWebUpdater();
  client.setMaxPacketSize(4096);
  // client.enableDebuggingMessages();

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

  for (int i = 0; i < NUMBER_OF_SPUNDERS; i++)
  {
    spunder_arr[i].name = SPUNDER_NAMES[i];
    spunder_arr[i].mqtt_field = MQTT_TEMP_FIELDS[i];
    spunder_arr[i].vols_setpoint = DESIRED_VOLS[i];
    spunder_arr[i].relay_pin = RELAY_PINS[i];
    spunder_arr[i].stored_time = millis();
    spunder_arr[i].ads_channel = i;
    spunder_arr[i].tempC         = JSON.stringify(parsed_data["data"][spunder_arr[i].mqtt_field]["value[degC]"]).toFloat();

    pinMode(spunder_arr[i].relay_pin, OUTPUT);
    digitalWrite(spunder_arr[i].relay_pin, !RELAY_OPEN);
  }
}

void onConnectionEstablished()
{
  client.subscribe(_SUBTOPIC, [](const String &payload)
  {
    // Get the JSON data of the sub_topic
    JSONVar data; 
    parsed_data = JSON.parse(payload);
    if (JSON.typeof(parsed_data) == "undefined") 
    {
      Serial.println("Parsing input failed!");
      return;
    }

    // Read each spunder in the array
    float raw_temp;
    for (int i = 0; i < NUMBER_OF_SPUNDERS; i++)
    { 
      // Parse message from HOST, get temp needed, filter outliers     
      raw_temp = JSON.stringify(parsed_data["data"][spunder_arr[i].mqtt_field]["value[degC]"]).toFloat();
      if ((spunder_arr[i].tempC - raw_temp) < .3) { spunder_arr[i].tempC = raw_temp; }   
      
      // Get data values
      spunder_arr[i].tempF           = spunder_arr[i].tempC * 1.8 + 32;
      spunder_arr[i].adc             = spunder_arr[i].get_adc();
      spunder_arr[i].volts           = spunder_arr[i].get_volts();
      spunder_arr[i].psi_value       = spunder_arr[i].get_psi_value();
      //spunder_arr[i].psi_smooth      = spunder_arr[i].get_psi_smooth();
      spunder_arr[i].psi_setpoint    = spunder_arr[i].get_psi_setpoint();
      spunder_arr[i].vols_value      = spunder_arr[i].get_vols();
      
      spunder_arr[i].test_carb();
  
      // Populate data message    
      data[spunder_arr[i].name]["adc"]          = spunder_arr[i].adc;
      data[spunder_arr[i].name]["volts"]        = spunder_arr[i].volts;
      data[spunder_arr[i].name]["tempC"]        = spunder_arr[i].tempC;
      data[spunder_arr[i].name]["psi_setpoint"] = spunder_arr[i].psi_setpoint;
      data[spunder_arr[i].name]["psi"]          = spunder_arr[i].psi_value;
      //data[spunder_arr[i].name]["smooth_psi"]   = spunder_arr[i].psi_smooth;
      data[spunder_arr[i].name]["bar_setpoint"] = spunder_arr[i].bar_setpoint;
      data[spunder_arr[i].name]["bar"]          = spunder_arr[i].bar_value;
      data[spunder_arr[i].name]["vols_target"]  = spunder_arr[i].vols_setpoint;
      data[spunder_arr[i].name]["volumes[co2]"] = spunder_arr[i].vols_value;
      data[spunder_arr[i].name]["since_vent"]   = spunder_arr[i].time_since_vent;
    }

    // Output
    Serial.println(JSON.stringify(data));
    delay(5000);
  });
}

void loop()
{
  client.loop();
}