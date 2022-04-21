#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include <Esp32HTTPUpdateServer.h>

#include "spunder_config.hpp"
#include "spunder.hpp"

#define NUMBER_OF_SPUNDERS 4
#define RELAY_OPEN HIGH

// Create array of Spunders
std::array<Spunder, NUMBER_OF_SPUNDERS> spund_arr;

JSONVar parsed_data;

// From spund_config.h
EspMQTTClient client(_SSID, _PASS, _MQTTHOST, _CLIENTID, _MQTTPORT);

void onConnectionEstablished(void);

void setup()
{
  Serial.begin(115200);
  client.setMaxPacketSize(4096);
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