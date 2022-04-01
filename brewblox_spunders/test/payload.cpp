#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include <Esp32HTTPUpdateServer.h>

#include "spund_config.hpp"
#include "spunder.hpp"

#define NUMBER_OF_SPUNDERS 4
#define RELAY_OPEN HIGH

// Create array of Spunders
Spunder spund_arr[NUMBER_OF_SPUNDERS];

JSONVar parsed_data; 

// From spund_config.h
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

void onConnectionEstablished(void);

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

}

void onConnectionEstablished()
{
  client.subscribe(_SUBTOPIC, [](const String &payload)
  {
    Serial.println(payload);
    Serial.println(payload.length());
  });

  delay(5000);
}

void loop()
{
  client.loop();
}