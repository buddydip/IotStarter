import re
from typing import NamedTuple

import json
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient

INFLUXDB_ADDRESS = 'Smarty.local'
INFLUXDB_USER = 'smarty'
INFLUXDB_PASSWORD = 'zyxcba123'
INFLUXDB_DATABASE = 'smarty'

MQTT_ADDRESS = 'Smarty.local'
MQTT_USER = 'pi'
MQTT_PASSWORD = 'meripi123'
MQTT_TOPIC_AMBIENCE: str = 'smarty/ambience'
MQTT_TOPIC_SWITCH: str = 'smarty/switchcontrol'
MQTT_CLIENT_ID = 'MQTTInfluxDBBridge'

influxdb_client = InfluxDBClient(INFLUXDB_ADDRESS, 8086, INFLUXDB_USER, INFLUXDB_PASSWORD, None)

class SensorData(NamedTuple):
    ParamName: str
    Time: str
    ParamValue: float

class SwitchData(NamedTuple):
    SwitchID: str
    Time: str
    SwitchState: bool

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC_AMBIENCE)
    client.subscribe(MQTT_TOPIC_SWITCH)

def _parse_ambience_message(topic, payload):
    if topic == MQTT_TOPIC_AMBIENCE:
        data = json.loads(payload)
        paramname = data['ParamName']
        if paramname == "Motion":
            paramvalue = bool(data['ParamValue'])
        elif paramname == "Sound" or paramname == "Humidity" or paramname == "Smoke" or paramname == "LPG" \
                or paramname == "CO2":
            paramvalue = int(data['ParamValue'])
        else:
            paramvalue = float(data['ParamValue'])

        time = data['Time']
        return SensorData(paramname, time, paramvalue)
    else:
        return None

def _parse_switch_message(topic, payload):
    if topic == MQTT_TOPIC_SWITCH:
        data = json.loads(payload)
        switchID = data['SwitchID']
        switchState = bool(data['SwitchState'])
        switchTime = data['Time']
        return SwitchData(switchID, switchTime, switchState)
    else:
        return None

def _send_sensor_data_to_influxdb(sensor_data):
    json_body = [
        {
            'measurement': sensor_data.ParamName,
            'tags': {
                'timestamp': sensor_data.Time
            },
            'fields': {
                'value': sensor_data.ParamValue
            }
        }
    ]
    influxdb_client.write_points(json_body)


def _send_switch_data_to_influxdb(switch_data):
    json_body = [
        {
            'measurement': switch_data.SwitchID,
            'tags': {
                'timestamp': switch_data.Time
            },
            'fields': {
                'value': switch_data.SwitchState
            }
        }
    ]
    influxdb_client.write_points(json_body)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))

    if msg.topic == MQTT_TOPIC_AMBIENCE and msg.payload is not None:
        sensor_data = _parse_ambience_message(msg.topic, msg.payload.decode('utf-8'))
        if sensor_data is not None:
            _send_sensor_data_to_influxdb(sensor_data)
    if msg.topic == MQTT_TOPIC_SWITCH and msg.payload is not None:
        switch_data = _parse_switch_message(msg.topic, msg.payload.decode('utf-8'))
        if switch_data is not None:
            _send_switch_data_to_influxdb(switch_data)



def _init_influxdb_database():
    databases = influxdb_client.get_list_database()
    if len(list(filter(lambda x: x['name'] == INFLUXDB_DATABASE, databases))) == 0:
        influxdb_client.create_database(INFLUXDB_DATABASE)
    influxdb_client.switch_database(INFLUXDB_DATABASE)

def main():
    _init_influxdb_database()

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()


if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()