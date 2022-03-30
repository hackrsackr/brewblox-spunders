#include <Arduino.h>
#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include <Esp32HTTPUpdateServer.h>
#include <math.h>

#include "secrets.hpp"
#include "spund_config.hpp"
#include "spunder.hpp"

#define NUMBER_OF_SPUNDERS 1
#define RELAY_OPEN HIGH

Spunder s;

JSONVar parsed_data; 

EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_ONE);
  client.enableHTTPWebUpdater();
  client.setMaxPacketSize(4096);
  client.enableDebuggingMessages();

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

  s.name          = _NAME1;
  s.mqtt_field    = _TEMP1;
  s.relay_pin     = _RPIN1;
  s.unit_max      = _UMAX1;
  s.vols_setpoint = _VOLS1;
  s.stored_time   = millis();
  s.ads_channel   = 0;
  s.tempC         = JSON.stringify(parsed_data["data"][s.mqtt_field]["value[degC]"]).toFloat();

  //Serial.println(s.tempC);
  pinMode(s.relay_pin, OUTPUT);
  digitalWrite(s.relay_pin, !RELAY_OPEN);
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
    float raw_temp;

      // Parse message from HOST, get temp needed, filter outliers     
    raw_temp = JSON.stringify(parsed_data["data"][s.mqtt_field]["value[degC]"]).toFloat();
    if ((s.tempC - raw_temp) < .3) { s.tempC = raw_temp; }   
    
    // Serial.println(s.tempC);
    // Get data values
    s.tempF           = s.convert_temp();
    //s.adc             = s.get_adc();
    //s.volts           = s.get_volts();
    s.psi_value       = s.get_psi_value();
    s.psi_setpoint    = s.get_psi_setpoint();
    s.vols_value      = s.get_vols();
    s.time_since_vent = s.test_carb();

    // Populate data message    
    //data[s.name]["adc"]          = s.adc;
    data[s.name]["volts"]        = s.volts;
    data[s.name]["tempC"]        = s.tempC;
    data[s.name]["psi_setpoint"] = s.psi_setpoint;
    data[s.name]["psi"]          = s.psi_value;
    data[s.name]["bar_setpoint"] = s.bar_setpoint;
    data[s.name]["bar"]          = s.bar_value;
    data[s.name]["vols_target"]  = s.vols_setpoint;
    data[s.name]["volumes[co2]"] = s.vols_value;
    data[s.name]["since_vent"]   = s.time_since_vent;

    //Serial.println(JSON.stringify(data));
    //Serial.println("");

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