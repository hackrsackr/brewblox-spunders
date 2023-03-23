// HTML web page to handle 4 input fields (Setpoint 1, Setpoint 2, Setpoint 3, Setpoint 4)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <title>Spunder Setpoints</title>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
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
        <p>Spunder Setpoints vols[co2]</p>
        <form action="/get">
            sepoint-1:
            <input
                type="number"
                step="0.01"
                name="setpoint_input1"
                value="%SETPOINT1%"
                required
            />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            sepoint-2:
            <input
                type="number"
                step="0.01"
                name="setpoint_input2"
                value="%SETPOINT2%"
                required
            />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            sepoint-3:
            <input
                type="number"
                step="0.01"
                name="setpoint_input3"
                value="%SETPOINT3%"
                required
            />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            sepoint-4:
            <input
                type="number"
                step="0.01"
                name="setpoint_input4"
                value="%SETPOINT4%"
                required
            />
            <input type="submit" value="Submit" />
        </form>
        <p>Spunder MQTT temp fields</p>
        <form action="/get">
            mqtt_temp_field-1:
            <input type="text" name="mqtt_input1" value="%MQTT1%" required />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            mqtt_temp_field-2:
            <input type="text" name="mqtt_input2" value="%MQTT2%" required />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            mqtt_temp_field-3:
            <input type="text" name="mqtt_input3" value="%MQTT3%" required />
            <input type="submit" value="Submit" />
        </form>
        <br />
        <form action="/get">
            mqtt_temp_field-4:
            <input type="text" name="mqtt_input4" value="%MQTT4%" required />
            <input type="submit" value="Submit" />
        </form>
    </body>
</html>)rawliteral";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

String processor(const String &var)
{
    if (var == "SETPOINT1")
    {
        return _SETPOINT_MESSAGES[0];
    }
    if (var == "SETPOINT2")
    {
        return _SETPOINT_MESSAGES[1];
    }
    if (var == "SETPOINT3")
    {
        return _SETPOINT_MESSAGES[2];
    }
    if (var == "SETPOINT4")
    {
        return _SETPOINT_MESSAGES[3];
    }
    if (var == "MQTT1")
    {
        return _MQTT_MESSAGES[0];
    }
    if (var == "MQTT2")
    {
        return _MQTT_MESSAGES[1];
    }
    if (var == "MQTT3")
    {
        return _MQTT_MESSAGES[2];
    }
    if (var == "MQTT4")
    {
        return _MQTT_MESSAGES[3];
    }
    return String();
}
