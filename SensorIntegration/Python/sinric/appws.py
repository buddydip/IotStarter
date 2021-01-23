from paho.mqtt.client import MQTT_CLIENT
from sinric import Sinric
from credentials import appKey, secretKey, deviceIdArr
from time import sleep
from datetime import datetime
import websocket
import json
import base64
import paho.mqtt.client as mqtt

MQTT_ADDRESS = 'Smarty.local'
MQTT_USER = 'pi'
MQTT_PASSWORD = 'meripi123'
MQTT_TOPIC_AMBIENCE = 'smarty/ambience'
MQTT_TOPIC_SWITCH = 'smarty/switchcontrol'
MQTT_CLIENT_ID = 'MQTTBotBridge'


def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))


def on_open(ws):
    print
    "### Initiating new websocket connection ###"

def on_message(ws, message):
    obj = json.loads(message)
    deviceId = obj['deviceId']
    action = obj['action']
    value = obj['value']

    print(message)

    print('Power Change')
    msg = {
        "SwitchID": "\""+deviceId+"\"",
        "SwitchState": action,
        "Time": "\""+datetime.now()+"\""
    }

    # convert into JSON:
    payload = json.dumps(msg)

    #mqtt_client.publish(MQTT_TOPIC_SWITCH, payload)
    print(payload)


def on_error(ws, error):
    print(error)


def on_close(ws):
    print
    "### closed ###"
    time.sleep(2)
    initiate()

def initiate():
    websocket.enableTrace(True)
    s = 'apikey:' + appKey

    ws = websocket.WebSocketApp("ws://iot.sinric.com",
                                header={'Authorization:' + 'apikey:fc916cbf-fe00-4e95-bc7c-c75916fd2b13'},
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.on_open = on_open
    ws.run_forever()

if __name__ == "__main__":
    initiate()
    print('Connected to Sinric ')

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_start()  # start the loop

