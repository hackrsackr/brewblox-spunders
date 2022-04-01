#include <Arduino.h>
#include <Arduino_JSON.h>
#include <EspMQTTClient.h>
#include <Esp32HTTPUpdateServer.h>

#include "secrets.hpp"
#include "spund_config.hpp"
#include "spunder.hpp"



void setup()
{
  Serial.begin(115200);
  ads.begin(0x48);
  ads.setGain(GAIN_ONE);
}


void loop()
{
  Serial.println(ads.readADC_SingleEnded(0));
  delay(5000);
}