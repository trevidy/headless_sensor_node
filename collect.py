import paho.mqtt.client as mqtt
import csv
import datetime
import os

TOPIC = "headless_node_tjcdy/telemetry"
FILE = "sensor_log.csv"

# callback function: runs the moment the script connects to the MQTT broker.
def on_connect(client, userdata, flags, rc):
    client.subscribe(TOPIC)
    print(f"Subscribed to {TOPIC}")

# callback function: runs every time a new piece of data arrives
def on_message(client, userdata, msg):
    payload = msg.payload.decode() # decode the incoming message from raw bytes into readable text
    timestamp = datetime.datetime.now().isoformat() # get the curent date and time from laptop
    print(f"{timestamp} | {payload}") 

    write_header = not os.path.exists(FILE) # flag if CSV file exists. 
    
    # open the csv file:
    # "a" stands for "append" mode, which safely adds new data to the bottom without erasing old data
    # 'newline = ""' prevents blank rows from being inserted between data
    with open(FILE, "a", newline = "") as f:
        writer = csv.writer(f) # hand over 'f' to python's built-in csv library tool called writer
        if write_header: # if new file, write the column header first"
            writer.writerow(["timestamp", "payload"])
        writer.writerow([timestamp, payload]) # write timestamp and sensor data into a new row.

# --- main script execution --- #
client = mqtt.Client() # create an instance of an MQTT client object
client.on_connect = on_connect # link created functions above to the client's internal event triggers
client.on_message = on_message 
client.connect("broker.hivemq.com", 1883) # connect to HiveMQ's free, public internet broker on standard port 1883
client.loop_forever()