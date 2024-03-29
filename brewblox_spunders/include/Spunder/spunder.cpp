#include <iostream>
#include <ArduinoJson.h>
#include <EspMQTTClient.h>
#include <ESP32HTTPUpdateServer.h>

#include "Spunder/spunder.hpp"
#include "Spunder/spunder_config.hpp"

// From spunder_config.h
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);
StaticJsonDocument<4096> input;
std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;

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

    // WiFi
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(_SSID, _PASS);
    uint8_t failed_connections = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("connecting..");
        failed_connections++;
        if (failed_connections > 20)
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
        spund_arr[spunder].name = SPUNDER_NAMES[spunder];
        spund_arr[spunder].mqtt_field = MQTT_FIELDS[spunder];
        spund_arr[spunder].vols_setpoint = DESIRED_VOLS[spunder];
        spund_arr[spunder].unit_max = UNIT_MAXS[spunder];
        spund_arr[spunder].relay_pin = RELAY_PINS[spunder];
        spund_arr[spunder].offset_volts = ESP_VOLTS[spunder];

        spund_arr[spunder].esp_vusb = _VESP;
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
        Serial.println(payload);
        deserializeJson(input, payload);
        for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
        {
          spund_arr[spunder].tempC = input["data"][spund_arr[spunder].mqtt_field]["value[degC]"];
        }
        publishData(); });
}

void publishData()
{
    StaticJsonDocument<768> message;
    message["key"] = _CLIENTID;

    // Read each spunder in the array of spunders
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
        if (!spund_arr[spunder].tempC)
        {
            Serial.println(" no temp reading");
            continue;
        }
        else
        {
            spund_arr[spunder].spunder_run();
        }

        // Populate data message to publish to brewblox
        message["data"][spund_arr[spunder].name]["volts"] = spund_arr[spunder].volts;
        message["data"][spund_arr[spunder].name]["tempC"] = spund_arr[spunder].tempC;
        message["data"][spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
        message["data"][spund_arr[spunder].name]["psi"] = spund_arr[spunder].psi_value;
        message["data"][spund_arr[spunder].name]["vols_target"] = spund_arr[spunder].vols_setpoint;
        message["data"][spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
        message["data"][spund_arr[spunder].name]["since_vent"] = spund_arr[spunder].time_since_vent;
        message["data"][spund_arr[spunder].name]["vent_state"] = spund_arr[spunder].vent_state;
    }

    serializeJsonPretty(message, Serial);

    client.publish(_PUBTOPIC, message.as<String>());

    delay(5000);
}

void loop()
{
    client.loop();
}
