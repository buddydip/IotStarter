# all configuration settings come from config.py
try:
	import config
except ImportError:
	print("Please copy template-config.py to config.py and configure appropriately !"); exit();

import json
import sys
import paho.mqtt.client as mqtt
import time
import random
import serial
import RPi.GPIO as GPIO       ## Import GPIO library


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

gpio_ledpin = 11


# for upstream communication
my_publish_topic      = "iot/data/" + my_device_id
# for downstream communication
my_subscription_topic = "iot/push/" + my_device_id

is_connected = False

def on_connect(mqttc, obj, flags, rc):
	print("on_connect   - rc: " + str(rc)+str(obj))
	global is_connected
	is_connected = True

# you can use the push API (e.g. also from the built-in sample UI) to send to the device
def on_message(mqttc, obj, msg):
	print("on_message   - " + msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
	json_input = str(msg.payload)
	
        try:
                decoded = json.loads(json_input)
                message = decoded['messages'][0]
                fanStatus = bool(message['fanStatus'])
                runDuration = int(message['runDuration'])
                print("FanStatus : "+str(fanStatus))
                print("RunDuration : " +str(runDuration))
        except (ValueError, KeyError, TypeError):
                print "JSON format error"

   	
        GPIO.setmode(GPIO.BOARD)        ## Use board pin numbering
        GPIO.setup(gpio_ledpin, GPIO.OUT)      ## Setup GPIO Pin 11 to OUT
        starttimestamp= time.time()
        GPIO.output(gpio_ledpin,False) ## Turn off Led
        print("Start : " +str(starttimestamp))
        while True:
                currenttime = time.time()
                print("Current :"+str(currenttime))
                duration = int(currenttime) - int(starttimestamp)
                print("Duration : "+str(duration))
                
                if duration > runDuration:
                        GPIO.output(gpio_ledpin,False) ## Turn off Led
                        break;      
                else:
                        GPIO.output(gpio_ledpin,True)  ## Turn on Led
                        time.sleep(1)         ## Wait for one second
                        GPIO.output(gpio_ledpin,False) ## Turn off Led
                        time.sleep(1)         ## Wait for one second
	


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

        my_mqtt_payload='{"mode":"async", "messageType":"' + str(config.message_type_id_From_device) + '", "messages":[{'+data+', "timestamp":"'+str(timestamp)+'"}]}'
        print("Topic :"+my_publish_topic+" Payload :"+my_mqtt_payload)
	res = mqttc.publish(my_publish_topic, my_mqtt_payload, qos=0)
	print(res)


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

# Connect the serial port
do_send=0
data=0
msg_string=""

#== Start Read Sensor Data from Serial Port ==#
arduinoSerialData = serial.Serial('/dev/ttyACM0',9600)
mqttc.loop_start()

publish_interval=10
value=0
while 1==1:

        oldData = data
        data=read_data(arduinoSerialData, data)
        print("Current value : "+str(data)+"Old value :"+str(oldData))
        if (data != oldData):
                send_to_hcp(data)
                do_send=1

	time.sleep(publish_interval)
