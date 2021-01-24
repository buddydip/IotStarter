from paho.mqtt.client import MQTT_CLIENT
from sinric import SinricPro
from sinric import SinricProUdp
from credentials import appKey, secretKey, deviceIdArr,deviceNameArr
from time import sleep
from datetime import datetime
import json
import paho.mqtt.client as mqtt

MQTT_ADDRESS = 'Smarty.local'
MQTT_USER = 'pi'
MQTT_PASSWORD = 'meripi123'
MQTT_TOPIC_AMBIENCE = 'smarty/ambience'
MQTT_TOPIC_SWITCH = 'smarty/switchcontrol'
MQTT_CLIENT_ID = 'SmartyVoiceBot'

mqtt_client = mqtt.Client(MQTT_CLIENT_ID)

def onPowerState(did, state):
    # Alexa, turn ON/OFF Device

    switchID = ""
    switchState = 0

    if (did == deviceIdArr[0]):
        switchID = deviceNameArr[0]
    if (did == deviceIdArr[1]):
        switchID = deviceNameArr[1]
    if (did == deviceIdArr[2]):
        switchID = deviceNameArr[2]
    if (did == deviceIdArr[3]):
        switchID = deviceNameArr[3]

    if (switchID == ''):
        print('Device id is missing')
        return

    if (state == "On"):
        switchState = 1
    else:
        switchState = 0

    nowtime = str(datetime.now())

    msg = {'SwitchID': switchID, 'SwitchState': switchState, 'Time': nowtime}
    payload = json.dumps(msg)

    mqtt_client.publish(MQTT_TOPIC_SWITCH, payload)
    print(payload)
    return True, state

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))


def Events():
    while True:
        # Select as per your requirements
        # REMOVE THE COMMENTS TO USE

        # client.event_handler.raiseEvent(tvId, 'setPowerState',data={'state': 'On'})
        pass


event_callback = {
    'Events': Events
}

eventsCallbacks={
    "Events": Events
}

callbacks = {
    'powerState': onPowerState
}

if __name__ == "__main__":

    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_start()  # start the loop


    client = SinricPro(appKey, deviceIdArr, callbacks,event_callbacks=eventsCallbacks,
                       enable_log=False,restore_states=True,secretKey=secretKey)
    udp_client = SinricProUdp(callbacks,deviceIdArr,enable_trace=False)
    # Set it to True to start logging request Offline Request/Response

    client.handle_all(udp_client)
    print('Connected to Sinric ')




