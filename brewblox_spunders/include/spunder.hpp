#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;

class Spunder
{
public:
    uint16_t adc;              // Analog Bits
    uint8_t ads_channel;       // Channel of the ADS1115 to read
    uint8_t relay_pin;         // Esp32 pin of the spunder valve relay
    uint8_t unit_max;          // Max pressure rating of the transducer in PSI

    double psi_setpoint;       // Setpoint in PSI
    double psi_value;          // Pressure in PSI
    double stored_time;        // Time of last vent
    double tempC;              // Temp in Celsius
    double tempF;              // Temp in Fahrenheit
    double time_since_vent;    // Time since last vent
    double vols_setpoint;      // Desired co2 in vols
    double vols_value;         // Actual co2 in vols
    double volts;              // Volts read by the ADS1115
    double esp_vusb;           // Actual voltage of the 5v intput
    double offset_volts;       // Actual voltage at zero psi

    String mqtt_field;         // MQTT temperature field
    String name;               // Name of spunder

    double get_psi_value();    // Compute psi value from volts
    double get_psi_setpoint(); // Compute psi setpoint from tempC and vols_setpoint
    double get_vols();         // Compute volumes CO2 from tempC and psi_value
    double test_carb();        // Test if psi_value is greater than psi_setpoint, vent if neccessary
    double convert_temp();     // Convert temp C to F

    void spunder_run();      // Get temp, psi, check carb, vent if neccessary

private:
    // ADS1115 methods
    uint16_t get_adc();       // Read adc value from ads1115
    double get_volts();       // Read volts value from ads1115
};

uint16_t Spunder::get_adc()
{
    adc = ads.readADC_SingleEnded(ads_channel);

    return adc;
}

double Spunder::get_volts()
{
    volts = ads.computeVolts(ads.readADC_SingleEnded(ads_channel));

    return volts;
}

double Spunder::get_psi_value()
{
    volts = ads.computeVolts(ads.readADC_SingleEnded(ads_channel));
    psi_value = (volts - offset_volts) * (unit_max / 4.0);

    return psi_value;
}

double Spunder::get_psi_setpoint()
{
    double a = -16.669 - (.0101059 * tempF) + (.00116512 * (tempF * tempF));
    double b = .173354 * tempF * vols_setpoint;
    double c = (4.24267 * vols_setpoint) - (.0684226 * (vols_setpoint * vols_setpoint));

    psi_setpoint = a + b + c;

    return psi_setpoint;
}

double Spunder::get_vols()
{
    double a = -.0684226;
    double b = ((.173354 * tempF) + 4.24267);
    double c = (-psi_value + -16.669 + (-0.0101059 * tempF) + (0.00116512 * tempF * tempF));
    double d = ((b * b) - (4 * a * c));

    vols_value = ((-b + (pow(d, .5))) / (2 * a));

    return vols_value;
}

double Spunder::test_carb()
{
    if (psi_value > psi_setpoint)
    {
        digitalWrite(relay_pin, HIGH);
        delay(500);
        digitalWrite(relay_pin, LOW);
        delay(500);
        stored_time = millis();
    }
    time_since_vent = ((millis() - stored_time) / 60000.0);

    return time_since_vent;
}

double Spunder::convert_temp()
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
