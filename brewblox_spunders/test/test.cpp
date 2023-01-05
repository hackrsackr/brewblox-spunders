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

AsyncWebServer server(80);

// HTML web page to handle 4 input fields (Setpoint 1, Setpoint 2, Setpoint 3, Setpoint 4)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Spunder Setpoints</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
    padding: 25px;
    background-color: black;
    color: grey;
    font-size: 25px;
    }
  </style></head>
  <body>
  <p>Spunder Setpoints vols[co2]</p>
  <form action="/get">
    sepoint-1: <input type="number" step="0.1" name="setpoint_input1" value=%SETPOINT1% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-2: <input type="number" step="0.1" name="setpoint_input2" value=%SETPOINT2% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-3: <input type="number" step="0.1" name="setpoint_input3" value=%SETPOINT3% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-4: <input type="number" step="0.1" name="setpoint_input4" value=%SETPOINT4% required>
    <input type="submit" value="Submit">
  </form>
  <p>Spunder MQTT temp fields</p>
  <form action="/get">
    mqtt_temp_field-1: <input type="text" name="mqtt_input1" value=%MQTT1% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    mqtt_temp_field-2: <input type="text" name="mqtt_input2" value=%MQTT2% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    mqtt_temp_field-3: <input type="text" name="mqtt_input3" value=%MQTT3% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    mqtt_temp_field-4: <input type="text" name="mqtt_input4" value=%MQTT4% required>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

String processor(const String &var)
{
    if (var == "SETPOINT1")
    {
        return setpointMessage1;
    }
    if (var == "SETPOINT2")
    {
        return setpointMessage2;
    }
    if (var == "SETPOINT3")
    {
        return setpointMessage3;
    }
    if (var == "SETPOINT4")
    {
        return setpointMessage4;
    }
    if (var == "MQTT1")
    {
        return mqttMessage1;
    }
    if (var == "MQTT2")
    {
        return mqttMessage2;
    }
    if (var == "MQTT3")
    {
        return mqttMessage3;
    }
    if (var == "MQTT4")
    {
        return mqttMessage4;
    }
    return String();
}

std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;

EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);
StaticJsonDocument<4096> input;

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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, processor); });
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String inputMessage;
    String inputParam;

    if (request->hasParam(_SETPOINT_INPUTS[0])) {
      // GET setpoint_input1 values on <ESP_IP>/get?setpoint_input1=<inputMessage>
      setpointMessage1 = request->getParam(_SETPOINT_INPUTS[0])->value();
      spund_arr[0].vols_setpoint = setpointMessage1.toFloat();
      inputMessage = setpointMessage1;
      inputParam = _SETPOINT_INPUTS[0];
    }

    else if (request->hasParam(_SETPOINT_INPUTS[1])) {
      // GET setpoint_input2 value on <ESP_IP>/get?setpoint_input2=<inputMessage>
      setpointMessage2 = request->getParam(_SETPOINT_INPUTS[1])->value();
      spund_arr[1].vols_setpoint = setpointMessage2.toFloat();
      inputMessage = setpointMessage2;
      inputParam = _SETPOINT_INPUTS[1];
    }
    else if (request->hasParam(_SETPOINT_INPUTS[2])) {
      // GET setpoint_input3 value on <ESP_IP>/get?setpoint_input3=<inputMessage>
      setpointMessage3 = request->getParam(_SETPOINT_INPUTS[2])->value();
      spund_arr[2].vols_setpoint = setpointMessage3.toFloat();
      inputMessage = setpointMessage3;
      inputParam = _SETPOINT_INPUTS[2];
    }
    else if (request->hasParam(_SETPOINT_INPUTS[3])) {
      // GET setpoint_input4 value on <ESP_IP>/get?setpoint_input4=<inputMessage>
      setpointMessage4 = request->getParam(_SETPOINT_INPUTS[3])->value();
      spund_arr[3].vols_setpoint = setpointMessage4.toFloat();
      inputMessage = setpointMessage4;
      inputParam = _SETPOINT_INPUTS[3];
    }
    else if (request->hasParam(_MQTT_INPUTS[0])) {
      // GET mqtt_input1 values on <ESP_IP>/get?mqtt_input1=<inputMessage>
      mqttMessage1 = request->getParam(_MQTT_INPUTS[0])->value();
      spund_arr[0].mqtt_field = mqttMessage1;
      inputMessage = mqttMessage1;
      inputParam = _MQTT_INPUTS[0];
    }
    else if (request->hasParam(_MQTT_INPUTS[1])) {
      // GET mqtt_input2 values on <ESP_IP>/get?mqtt_input2=<inputMessage>
      mqttMessage2 = request->getParam(_MQTT_INPUTS[1])->value();
      spund_arr[1].mqtt_field = mqttMessage2;
      inputMessage = mqttMessage2;
      inputParam = _MQTT_INPUTS[1];
    }
    else if (request->hasParam(_MQTT_INPUTS[2])) {
      // GET mqtt_input3 values on <ESP_IP>/get?mqtt_input3=<inputMessage>
      mqttMessage3 = request->getParam(_MQTT_INPUTS[2])->value();
      spund_arr[2].mqtt_field = mqttMessage3;
      inputMessage = mqttMessage3;
      inputParam = _MQTT_INPUTS[2];
    }
    else if (request->hasParam(_MQTT_INPUTS[3])) {
      // GET mqtt_input4 values on <ESP_IP>/get?mqtt_input4=<inputMessage>
      mqttMessage4 = request->getParam(_MQTT_INPUTS[3])->value();
      spund_arr[3].mqtt_field = mqttMessage4;
      inputMessage = mqttMessage4;
      inputParam = _MQTT_INPUTS[3];
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
