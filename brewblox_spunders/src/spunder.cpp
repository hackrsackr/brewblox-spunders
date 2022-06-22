#include <iostream>

#include <ArduinoJson.h>
#include <EspMQTTClient.h>
#include <ESP32HTTPUpdateServer.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>

#include "spunder_config.hpp"
#include "spunder.hpp"

AsyncWebServer server(80);
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);
std::array<Spunder, _NUMBER_OF_SPUNDERS> spund_arr;

DynamicJsonDocument input(4096);

// Client run function
void onConnectionEstablished(void);
void publishData(void);
String processor(const String&);
void notFound(AsyncWebServerRequest*);

String setpoint_input = String(DESIRED_VOLS[0]);
String enabled_input = "true";
String enableArmChecked = "";
const char *PARAM_INPUT_1 = "setpoint_input";
const char *PARAM_INPUT_2 = "enable_arm_input";

float psi = 15.00;

// HTML web page to handle 2 input fields (setpoint_input, enabled_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
<title>Volume Level Control</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {
  padding: 25px;
  background-color: black;
  color: grey;
  font-size: 25px;
}
</style>
</head>
<body>
<p><strong>KettleFiller</strong></p>
<form action="/get">
  Enabled <input type="checkbox" name="enable_arm_input" value="true" %ENABLE_ARM_INPUT%><br><br>
  Setpoint[L] <input type="number" step="0.5" name="setpoint_input" value="%SETPOINT%" required><br>
  <input type="submit" value="Submit">
</form>
<h4>Setpoint %SETPOINT% liters</h4>
<h4>Volume %VOLUME% liters</h4>
</body></html>)rawliteral";

void setup()
{
    Serial.begin(115200);
    // ADS
    ads.begin(0x48);
    ads.setGain(GAIN_TWOTHIRDS);
    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1);
    }
    //client.enableHTTPWebUpdater();
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
        if (failed_connections > 20)
        {
            Serial.println("restarting..");
            ESP.restart();
        }
    }
    Serial.print("Connected to ");
    Serial.println(WiFi.localIP());

    // Server
    //Send web page to client
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
    });
    // Receive an HTTP GET request at <ESP_IP>/get?setpoint_input=<setpoint_input>&enable_arm_input=<enabled_input>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      setpoint_input = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_arm_input value on <ESP_IP>/get?enable_arm_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        enabled_input = request->getParam(PARAM_INPUT_2)->value();
        enableArmChecked = "checked";
      }
      else {
        enabled_input = "false";
        enableArmChecked = "";
      }
    }
    //kf.kf_enabled = enabled_input;
    Serial.println(setpoint_input);
    Serial.println(enabled_input);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
    });
    server.onNotFound(notFound);
    server.begin();


    // Spunder setup
    for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
    {
        spund_arr[spunder].name          = SPUNDER_NAMES[spunder];
        spund_arr[spunder].mqtt_field    = MQTT_FIELDS[spunder];
        spund_arr[spunder].vols_setpoint = DESIRED_VOLS[spunder];
        spund_arr[spunder].unit_max      = UNIT_MAXS[spunder];
        spund_arr[spunder].relay_pin     = RELAY_PINS[spunder];
        spund_arr[spunder].offset_volts  = ESP_VOLTS[spunder];

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
          //Serial.println(spund_arr[spunder].tempC);
        }
        publishData();
    });
}


void publishData()
{
    if (client.isConnected())
    {

        DynamicJsonDocument message(768);
        message["key"] = _CLIENTID;

        // Read each spunder in the array of spunders
        for (uint8_t spunder = 0; spunder < _NUMBER_OF_SPUNDERS; spunder++)
        {
            if (!spund_arr[spunder].tempC) {
                Serial.println(" no temp reading");
                continue;

            } else {
                spund_arr[spunder].spunder_run();
            }

            // Populate data message to publish to brewblox
            message["data"][spund_arr[spunder].name]["volts"]        = spund_arr[spunder].volts;
            message["data"][spund_arr[spunder].name]["tempC"]        = spund_arr[spunder].tempC;
            message["data"][spund_arr[spunder].name]["psi_setpoint"] = spund_arr[spunder].psi_setpoint;
            message["data"][spund_arr[spunder].name]["psi"]          = spund_arr[spunder].psi_value;
            message["data"][spund_arr[spunder].name]["vols_target"]  = spund_arr[spunder].vols_setpoint;
            message["data"][spund_arr[spunder].name]["volumes[co2]"] = spund_arr[spunder].vols_value;
            message["data"][spund_arr[spunder].name]["since_vent"]   = spund_arr[spunder].time_since_vent;
        }

        serializeJsonPretty(message, Serial);

        client.publish(_PUBTOPIC, message.as<String>());

        delay(5000);
    }
}

void loop()
{
    client.loop();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "VOLUME"){
    return String(psi);
  }
  else if(var == "SETPOINT"){
    spund_arr[0].vols_setpoint = setpoint_input.toFloat();
    return setpoint_input;
  }
  else if(var == "ENABLE_ARM_INPUT"){
    return enableArmChecked;
  }
  return String();
}
