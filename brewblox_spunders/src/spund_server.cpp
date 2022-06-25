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
String inputMessage1 = String(DESIRED_VOLS[0]);
String inputMessage2 = String(DESIRED_VOLS[1]);
String inputMessage3 = String(DESIRED_VOLS[2]);
String inputMessage4 = String(DESIRED_VOLS[3]);
const char *PARAM_INPUT_1 = "input1";
const char *PARAM_INPUT_2 = "input2";
const char *PARAM_INPUT_3 = "input3";
const char *PARAM_INPUT_4 = "input4";

// HTML web page to handle 3 input fields (input1, input2, input3)
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
    sepoint-1: <input type="text" name="input1" value=%SETPOINT1% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-2: <input type="text" name="input2" value=%SETPOINT2% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-3: <input type="text" name="input3" value=%SETPOINT3% required>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    sepoint-4: <input type="text" name="input4" value=%SETPOINT4% required>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) { 
  request->send(404, "text/plain", "Not found"); 
}

String processor(const String &var) {
  // Serial.println(var);
  if (var == "SETPOINT1") { return inputMessage1; }
  if (var == "SETPOINT2") { return inputMessage2; }
  if (var == "SETPOINT3") { return inputMessage3; }
  if (var == "SETPOINT4") { return inputMessage4; }
  return String();
}

std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;

EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);
//DynamicJsonDocument input(4096);
StaticJsonDocument<4096> input;

void onConnectionEstablished(void);
void publishData(void);

void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_TWOTHIRDS);

  client.setMaxPacketSize(4096);
  client.enableOTA();
  // client.enableDebuggingMessages();

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      spund_arr[0].vols_setpoint = inputMessage1.toFloat();
      inputMessage = inputMessage1;
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      spund_arr[1].vols_setpoint = inputMessage2.toFloat();
      inputMessage = inputMessage2;
      inputParam = PARAM_INPUT_2;
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      spund_arr[2].vols_setpoint = inputMessage3.toFloat();
      inputMessage = inputMessage3;
      inputParam = PARAM_INPUT_3;
    }
    else if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage4 = request->getParam(PARAM_INPUT_4)->value();
      spund_arr[3].vols_setpoint = inputMessage4.toFloat();
      inputMessage = inputMessage4;
      inputParam = PARAM_INPUT_4;
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>"); });
  server.onNotFound(notFound);
  server.begin();
}

void loop() { client.loop(); }

void onConnectionEstablished()
{
  client.subscribe(_SUBTOPIC, [](const String &payload)
  {
    Serial.println(payload);
    deserializeJson(input, payload);
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
    spund_arr[spunder].tempC = input["data"][spund_arr[spunder].mqtt_field]["value[degC]"];
    //std::cout << spund_arr[spunder].tempC << std::endl;
    }
    publishData(); });
}

void publishData()
{
  if (client.isConnected())
  {
    StaticJsonDocument<768> message;
    //DynamicJsonDocument message(768);
    message["key"] = _CLIENTID;

    // Read each spunder in the array of spunders
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
      if (!spund_arr[spunder].tempC)
      {
        Serial.println(" no temp reading");
        continue;
      } else { spund_arr[spunder].spunder_run(); }

      message["data"][spund_arr[spunder].name]["volts"] = spund_arr[spunder].volts;
      message["data"][spund_arr[spunder].name]["tempC"] = spund_arr[spunder].tempC;
      message["data"][spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
      message["data"][spund_arr[spunder].name]["psi"] = spund_arr[spunder].psi_value;
      message["data"][spund_arr[spunder].name]["vols_target"] = spund_arr[spunder].vols_setpoint;
      message["data"][spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
      message["data"][spund_arr[spunder].name]["since_vent"] = spund_arr[spunder].time_since_vent;
    }
    serializeJsonPretty(message, Serial);
    client.publish(_PUBTOPIC, message.as<String>());
    delay(5000);
  }
}

