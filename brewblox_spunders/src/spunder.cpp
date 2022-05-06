#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include "ESP32HTTPUpdateServer.h"

#include "spunder_config.hpp"
#include "spunder.hpp"

// From spunder_config.h
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
    ads.setGain(GAIN_TWOTHIRDS);

    client.enableHTTPWebUpdater();
    client.setMaxPacketSize(4096);
    client.enableOTA();
    //client.enableDebuggingMessages();

    // WiFi
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(_SSID, _PASS);
    
    uint8_t failed_connections = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("connecting..");
        failed_connections ++;
        if (failed_connections > 150) 
        {
            Serial.println("restarting..");
            ESP.restart();
        }
    }
    
    Serial.print("Connected to ");
    Serial.println(WiFi.localIP());

    // Spunder setup
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
        spund_arr[spunder].name          = SPUNDER_NAMES[spunder];
        spund_arr[spunder].mqtt_field    = MQTT_FIELDS[spunder];
        spund_arr[spunder].vols_setpoint = DESIRED_VOLS[spunder];
        spund_arr[spunder].unit_max      = UNIT_MAXS[spunder];
        spund_arr[spunder].relay_pin     = RELAY_PINS[spunder];
        spund_arr[spunder].offset_volts  = OFFSET_VOLTS[spunder];

        spund_arr[spunder].esp_vusb = _VUSB;
        spund_arr[spunder].stored_time = millis();
        spund_arr[spunder].ads_channel = spunder;

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
        publishData();
    });
}


void publishData()
{
    if (client.isConnected())
    {
        // Dictionaries to hold spunder data
        JSONVar data;
        JSONVar message;

        // Read each spunder in the array of spunders
        for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
        {
            // Parse message from _MQTTHOST to get temperature value needed
            spund_arr[spunder].tempC = JSON.stringify(parsed_data["data"][spund_arr[spunder].mqtt_field]["value[degC]"]).toFloat();
            if (!spund_arr[spunder].tempC) {
                Serial.print(spunder);
                Serial.println(" no temp reading");
                continue;

            } else {
                spund_arr[spunder].spunder_run();
            }

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
        message["key"] = "spunders";
        message["data"] = data;

        client.publish(_PUBTOPIC, JSON.stringify(message));
        
        Serial.println(message);
        Serial.println("");

        delay(5000);
    }
}

void loop()
{
    client.loop();
    //publishData();
}
