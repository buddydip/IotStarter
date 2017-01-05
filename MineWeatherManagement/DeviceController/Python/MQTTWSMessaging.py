# all configuration settings come from config.py
try:
	import config
except ImportError:
	print("Please copy template-config.py to config.py and configure appropriately !"); exit();

import sys
import paho.mqtt.client as mqtt
import time
import random
import serial


# === START === values set from the config.py file ===
my_endpoint             = "iotmms" + config.hcp_account_id + config.hcp_landscape_host
my_endpoint_certificate = config.endpoint_certificate
my_device_id            = config.device_id

my_client_id            = my_device_id
my_username             = my_device_id
my_password             = config.oauth_credentials_for_device

my_message_type_upstream = config.message_type_id_From_device
# === END ===== values set from the config.py file ===

my_endpoint_url_path1 = "/com.sap.iotservices.mms/v1/api/ws/mqtt"


# for upstream communication
my_publish_topic      = "iot/data/" + my_device_id
# for downstream communication
my_subscription_topic = "iot/push/" + my_device_id

is_connected = False

def on_connect(mqttc, obj, flags, rc):
	print("on_connect   - rc: " + str(rc))
	global is_connected
	is_connected = True

# you can use the push API (e.g. also from the built-in sample UI) to send to the device
def on_message(mqttc, obj, msg):
	print("on_message   - " + msg.topic + " " + str(msg.qos) + " " + str(msg.payload))

def on_publish(mqttc, obj, message_id):
	print("on_publish   - message_id: " + str(message_id))

def on_subscribe(mqttc, obj, message_id, granted_qos):
	print("on_subscribe - message_id: " + str(message_id) + " / qos: " + str(granted_qos))

def on_log(mqttc, obj, level, string):
	print(string)

def read_data(port, value):
	try:
                if(port.inWaiting()>0):
                        value = port.readline()
	except IOError:
		print("IOError communicating with HW - can and will continue though.")
		
	return(value)

def send_to_hcp(data):
        timestamp=int(time.time())

        my_mqtt_payload='{"mode":"async", "messageType":"' + str(config.message_type_id_From_device) + '", "messages":[{'+data+', "TimeStamp":"'+str(timestamp)+'"}]}'
        print(my_mqtt_payload)
	mqttc.publish(my_publish_topic, my_mqtt_payload, qos=0)


def connect_to_hcp():
        try:
                mqttc = mqtt.Client(client_id=my_client_id, transport='websockets')
                mqttc.on_message = on_message
                mqttc.on_connect = on_connect
                mqttc.on_publish = on_publish
                mqttc.on_subscribe = on_subscribe

                mqttc.tls_set(my_endpoint_certificate)
                mqttc.username_pw_set(my_username, my_password)
                mqttc.endpoint_url_path_set(my_endpoint_url_path1)
                print(my_endpoint)
                mqttc.connect(my_endpoint, 443, 60)

                # you can use the push API (e.g. also from the built-in sample UI) to send to the device
                mqttc.subscribe(my_subscription_topic, 0)
        except:
                print("Error in connecting to HCP")
                
        return(mqttc)



do_send=0
data=0
msg_string=""

# Connect the serial port
arduinoSerialData = serial.Serial('/dev/ttyACM0',9600)

#Connect to HCP#
mqttc = connect_to_hcp()
mqttc.loop_start()

publish_interval=10
value=0
while 1==1:

        oldData = data
        #== Start Read Sensor Data from Serial Port ==#
        data=read_data(arduinoSerialData, data)
        print("Current value : "+str(data)+"Old value :"+str(oldData))
        if (data != oldData):
                send_to_hcp(data)
                do_send=1

	time.sleep(publish_interval)
