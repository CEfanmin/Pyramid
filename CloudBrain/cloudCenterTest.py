import paho.mqtt.client as mqtt
import time,sys,random
import socket,sys
from thread import *
import threading,requests
from kafka import KafkaProducer

keep_alive=60
CLEAN_SESSION=False
port=1883
broker="localhost"

def loadExoTopics(inputFile):
    exoTopics = []
    with open(inputFile, 'r') as fin:
        lines = fin.readlines()
        for line in lines:
            lineR = line.strip()
            exoTopics.append(lineR)
    return exoTopics

def loadExoClients(inputFile):
    clients = []
    with open(inputFile, 'r') as fin:
        lines = fin.readlines()
        for line in lines:
            client = line.strip()
            clients.append(client.strip())
        return clients

def on_disconnect(client, userdata, flags, rc=0):
    m="DisConnected flags "+"result code "+str(rc)+" client_id"
    print(m)
    client.connected_flag=False

def on_connect(client, userdata, flags, rc):
    if rc==0:
        print("connected OK Returned code=",rc)
        client.connected_flag=True          # Flag to indicate success
    else:
        print("Bad connection Returned code=",rc)
        client.bad_connection_flag=True

tempStep = None
def toDataBase(clientID, clientStep):
        global tempStep
        if tempStep != clientStep:
                dynamicData = {'exoID':clientID,'exoStep':clientStep}
                try:
                        print ('clientID:',clientID)
                        print ('clientStep:',clientStep)
                        sendData = requests.post("http://localhost:9999/StepRecord", data=dynamicData)
                        tempStep = clientStep
                except:
                        print ("fail to sent to database!\n")


producer = KafkaProducer(bootstrap_servers='localhost:9092')
def on_message(client, userdata, message):
    print ("message received: " ,str(message.payload))
    messageStr=str(message.payload)
    messageDict=messageStr.split('=')
    if messageDict[0] == 'stick':
        producer.send(client._client_id +'_Stick', value=str(messageDict[1]))
    if messageDict[0] == 'battery':
        producer.send(client._client_id +'_DeviceBattery', value=str(messageDict[1]))
    if messageDict[0] == 'joint':
        producer.send(client._client_id +'_JointCode', value=str(messageDict[1]))
    if messageDict[0] == 'exobaseinfo':
        producer.send(client._client_id +'_ExoBaseInfo', value=str(messageDict[1]))
    if messageDict[0] == 'backattitude':
        producer.send(client._client_id +'_BackAttitude', value=str(messageDict[1]))
    if messageDict[0] == 'systeminfo':
		producer.send(client._client_id+'_SystemInfo', value=str(messageDict[1]))
		systemInfo = str(messageDict[1])
		Dic_SystemInfo = eval(systemInfo)
		client_Step = Dic_SystemInfo['walk_steps']
		toDataBase(client._client_id, client_Step)


def mainRun(client, exoTopic):
    run_main=False
    run_flag=True
    while run_flag:
        while not client.connected_flag and client.retry_count<3:
            count=0 
            run_main=False
            try:
                print("connecting ",broker)
                client.connect(broker,port,keep_alive)     #connect to broker
                break                   
            except:
                print("connection attempt failed will retry")
                client.retry_count += 1
                if client.retry_count > 3:
                    run_flag=False
        if not run_main:   
            client.loop_start()
            while True:
                if client.connected_flag:       #wait for connack
                    client.retry_count=0        #reset counter
                    run_main=True
                    break
                if count > 10 or client.bad_connection_flag:     # don't wait forever, last 10 seconds
                    client.loop_stop()                           # stop loop
                    client.retry_count+=1
                    if client.retry_count>3:
                        run_flag=False
                    break 

                time.sleep(1)
                count += 1
        if run_main:
            try:
                '''Do main loop'''
                # print("in main loop")     #publish and subscribe
                client.subscribe(exoTopic,0)
                time.sleep(1)             # control the speed

            except(KeyboardInterrupt):
                print("keyboard Interrupt so ending")
                run_flag=False
           
    print("Quit")
    client.disconnect()
    client.loop_stop()


def main():
    exoTopics = loadExoTopics('exoTopics')
    clients = loadExoClients('exoClients')
    for i in range(len(clients)):
        clients[i] = mqtt.Client(client_id=clients[i])
        clients[i].connected_flag=False         
        clients[i].bad_connection_flag=False 
        clients[i].retry_count=0 
        clients[i].on_connect=on_connect        # function callback
        clients[i].on_message = on_message
        clients[i].on_disconnect=on_disconnect
        threadx = threading.Thread(target=mainRun, args=(clients[i], exoTopics[i]))
        threadx.start()
        print ('thread'+str(i)+'  started!\n')
        time.sleep(1)

if __name__ == '__main__':
    main()

