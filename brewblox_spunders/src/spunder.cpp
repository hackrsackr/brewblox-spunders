#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include "Esp32HTTPUpdateServer.h"

#include "spunder_config.hpp"
#include "spunder.hpp"

// From spund_config.h
// Create a MQTT client
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

// Create array of Spunders
std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;

// Json object to hold the payload from client.suscribe
JSONVar parsed_data;

// Client run function
void onConnectionEstablished(void);
void publishData(void);

void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_ONE);

  client.enableHTTPWebUpdater();
  client.setMaxPacketSize(4096);
  client.enableOTA();
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

  // Spunder setup
  for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
  {
    spund_arr[spunder].name          = SPUNDER_NAMES[spunder];
    spund_arr[spunder].mqtt_field    = MQTT_FIELDS[spunder];
    spund_arr[spunder].vols_setpoint = DESIRED_VOLS[spunder];
    spund_arr[spunder].unit_max      = UNIT_MAXS[spunder];
    spund_arr[spunder].relay_pin     = RELAY_PINS[spunder];

    spund_arr[spunder].esp_vusb      = _VUSB;
    spund_arr[spunder].stored_time   = millis();
    spund_arr[spunder].ads_channel   = spunder;
    spund_arr[spunder].tempC         = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();

    pinMode(spund_arr[spunder].relay_pin, OUTPUT);
    digitalWrite(spund_arr[spunder].relay_pin, !_RELAY_OPEN);
  }
}

void onConnectionEstablished()
{
    client.subscribe(_SUBTOPIC, [](const String &payload)
    {
        // Get the JSON data of the sub_topic
        parsed_data = JSON.parse(payload);
        if (JSON.typeof(parsed_data) == "undefined")
        {
            Serial.println("Parsing input failed!");
            return;
        }
    });
}
void publishData()
{
    if (client.isConnected()){
        // Dictionaries to hold spunder data
        JSONVar data;
        JSONVar message;

        // Read each spunder in the array of spunders
        for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
        {
            // Parse message from _MQTTHOST to get temperature value needed
            //float new_temp = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();
            spund_arr[spunder].tempC = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();
            //spund_arr[spunder].convert_temp();
            spund_arr[spunder].spunder_run();

            //  Filter outliers
            //if ((spund_arr[spunder].tempC - new_temp) < .3) { spund_arr[spunder].tempC = new_temp; }

            // Convert analog reading and temp into psi_setpoint, psi_value, and current vols_value
            spund_arr[spunder].get_psi_setpoint();
            spund_arr[spunder].get_psi_value();
            spund_arr[spunder].get_vols();
            spund_arr[spunder].test_carb();

            // Populate data message to publish to brewblox
            data[spund_arr[spunder].name]["volts"]        = spund_arr[spunder].volts;
            data[spund_arr[spunder].name]["tempC"]        = spund_arr[spunder].tempC;
            data[spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
            data[spund_arr[spunder].name]["psi"]          = spund_arr[spunder].psi_value;
            data[spund_arr[spunder].name]["vols_target"]  = spund_arr[spunder].vols_setpoint;
            data[spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
            data[spund_arr[spunder].name]["since_vent"]   = spund_arr[spunder].time_since_vent;
        }

        // Format output into brewblox spec and publish
        message["key"]  = "spunders";
        message["data"] = data;

        client.publish(_PUBTOPIC, JSON.stringify(message));
        delay(5000);
    }
}

void loop()
{
  client.loop();
  publishData();
}
