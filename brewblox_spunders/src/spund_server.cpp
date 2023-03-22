#include <iostream>

#include <ArduinoJson.h>
#include <EspMQTTClient.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>

#include "Spunder/spunder.hpp"
#include "Spunder/spunder_config.hpp"
#include "Server/server.hpp"

std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;
StaticJsonDocument<4096> input;
AsyncWebServer server(80);
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

void initWifi(void);
void onConnectionEstablished(void);
void publishData(void);

void setup()
{
    Serial.begin(115200);
    ads.begin(0x48);
    ads.setGain(GAIN_TWOTHIRDS);

    client.setMaxPacketSize(4096);
    client.enableOTA();

    // WiFi
    initWifi();

    // Spunder setup
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
        spund_arr[spunder].name = _SPUNDER_NAMES[spunder];
        spund_arr[spunder].mqtt_field = _MQTT_FIELDS[spunder];
        spund_arr[spunder].vols_setpoint = _DESIRED_VOLS[spunder];
        spund_arr[spunder].unit_max = _UNIT_MAXS[spunder];
        spund_arr[spunder].relay_pin = _RELAY_PINS[spunder];
        spund_arr[spunder].offset_volts = _ESP_VOLTS[spunder];

        spund_arr[spunder].esp_vusb = _VESP;
        spund_arr[spunder].stored_time = millis();
        spund_arr[spunder].ads_channel = spunder;

        pinMode(spund_arr[spunder].relay_pin, OUTPUT);
        digitalWrite(spund_arr[spunder].relay_pin, !_RELAY_OPEN);
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, processor); });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String inputMessage;
    String inputParam;

    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
        if (request->hasParam(_SETPOINT_INPUTS[spunder])) {
            _SETPOINT_MESSAGES[spunder] = request->getParam(_SETPOINT_INPUTS[spunder])->value();
            spund_arr[spunder].vols_setpoint = _SETPOINT_MESSAGES[spunder].toFloat();
            inputMessage = _SETPOINT_MESSAGES[spunder];
            inputParam = _SETPOINT_INPUTS[spunder];
        }
        if (request->hasParam(_MQTT_INPUTS[spunder])) {
            _MQTT_MESSAGES[spunder] = request->getParam(_MQTT_INPUTS[spunder])->value();
            spund_arr[spunder].mqtt_field =  _MQTT_MESSAGES[spunder];
            inputMessage = _MQTT_MESSAGES[spunder];
            inputParam = _MQTT_INPUTS[spunder];
        }

    }

    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>"); });
    server.onNotFound(notFound);
    server.begin();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        initWifi();
    }
    client.loop();
}

void initWifi()
{
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
}

void onConnectionEstablished()
{
    client.subscribe(_SUBTOPIC, [](const String &payload)
                     {
    // Serial.println(payload);
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
    
    if (!client.isConnected()) {
        ESP.restart();
    }

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
            
            message["data"][spund_arr[spunder].name]["volts"] = spund_arr[spunder].volts;
            message["data"][spund_arr[spunder].name]["tempC"] = spund_arr[spunder].tempC;
            message["data"][spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
            message["data"][spund_arr[spunder].name]["psi"] = spund_arr[spunder].psi_value;
            message["data"][spund_arr[spunder].name]["vols_target"] = spund_arr[spunder].vols_setpoint;
            message["data"][spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
            message["data"][spund_arr[spunder].name]["since_vent"] = spund_arr[spunder].time_since_vent;
            message["data"][spund_arr[spunder].name]["vent_state"] = spund_arr[spunder].vent_state;
        }
    }    
    serializeJsonPretty(message, Serial);
    client.publish(_PUBTOPIC, message.as<String>());
    delay(5000);
    
}
