import serial
import json

from paho.mqtt import client as mqtt


HOST = '192.168.1.2'
PORT = 80
USB = '/dev/ttyUSB0'
HISTORY_TOPIC = 'brewcast/history'
TOPIC = HISTORY_TOPIC + '/spunders'

client = mqtt.Client(transport='websockets')
client.ws_set_options(path='/eventbus')

ser = serial.Serial(port=USB,
                    baudrate=115200,
                    timeout=1)

try:
    client.connect_async(host=HOST, port=PORT)
    client.loop_start()

    while True:
        message = ser.readline().decode()
        #print('decoded', message)
        
        try:
            message = json.loads(message)
        except json.JSONDecodeError:
            continue
        #print('json', message)
        
        client.publish(TOPIC, json.dumps(message))
        print(json.dumps(message, sort_keys=False, indent=4))

finally:
    ser.close()
    client.loop_stop
