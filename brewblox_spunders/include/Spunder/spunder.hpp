#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

class Spunder
{
public:
    uint16_t adc;        // Analog Bits
    uint8_t ads_channel; // Channel of the ADS1115 to read
    uint8_t relay_pin;   // Esp32 pin of the spunder valve relay
    uint8_t unit_max;    // Max pressure rating of the transducer in PSI
    uint8_t vent_state;  // State of vent relay

    float psi_setpoint;    // Setpoint in PSI
    float psi_value;       // Pressure in PSI
    float stored_time;     // Time of last vent
    float tempC;           // Temp in Celsius
    float tempF;           // Temp in Fahrenheit
    float time_since_vent; // Time since last vent
    float vols_setpoint;   // Desired co2 in vols
    float vols_value;      // Actual co2 in vols
    float volts;           // Volts read by the ADS1115
    float esp_vusb;        // Actual voltage of the 5v intput
    float offset_volts;    // Actual voltage at zero psi

    String mqtt_field; // MQTT temperature field
    String name;       // Name of spunder

    float get_psi_value();    // Compute psi value from volts
    float get_psi_setpoint(); // Compute psi setpoint from tempC and vols_setpoint
    float get_vols();         // Compute volumes CO2 from tempC and psi_value
    float test_carb();        // Test if psi_value is greater than psi_setpoint, vent if neccessary
    float convert_temp();     // Convert temp C to F

    void spunder_run(); // Get temp, psi, check carb, vent if neccessary

private:
    // ADS1115 methods
    uint16_t get_adc(); // Read adc value from ads1115
    float get_volts();  // Read volts value from ads1115
};

uint16_t Spunder::get_adc()
{
    adc = ads.readADC_SingleEnded(ads_channel);

    return adc;
}

float Spunder::get_volts()
{
    volts = ads.computeVolts(ads.readADC_SingleEnded(ads_channel));

    return volts;
}

float Spunder::get_psi_value()
{
    volts = ads.computeVolts(ads.readADC_SingleEnded(ads_channel));
    psi_value = (volts - offset_volts) * (unit_max / 4.0);

    return psi_value;
}

float Spunder::get_psi_setpoint()
{
    float a = -16.669 - (.0101059 * tempF) + (.00116512 * (tempF * tempF));
    float b = .173354 * tempF * vols_setpoint;
    float c = (4.24267 * vols_setpoint) - (.0684226 * (vols_setpoint * vols_setpoint));

    psi_setpoint = a + b + c;

    return psi_setpoint;
}

float Spunder::get_vols()
{
    float a = -.0684226;
    float b = ((.173354 * tempF) + 4.24267);
    float c = (-psi_value + -16.669 + (-0.0101059 * tempF) + (0.00116512 * tempF * tempF));
    float d = ((b * b) - (4 * a * c));

    vols_value = ((-b + (pow(d, .5))) / (2 * a));

    return vols_value;
}

float Spunder::test_carb()
{
    vent_state = 0;
    // if (psi_value > psi_setpoint)
    if (vols_value > vols_setpoint)
    {
        digitalWrite(relay_pin, HIGH);
        delay(500);
        digitalWrite(relay_pin, LOW);
        delay(500);
        stored_time = millis();
        vent_state = 1;
    }
    time_since_vent = ((millis() - stored_time) / 60000.0);

    return time_since_vent;
}

float Spunder::convert_temp()
{
    tempF = tempC * 1.8 + 32;

    return tempF;
}

void Spunder::spunder_run()
{
    tempF = convert_temp();
    psi_setpoint = get_psi_setpoint();
    psi_value = get_psi_value();
    vols_value = get_vols();
    time_since_vent = test_carb();
}
