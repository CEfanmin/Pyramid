# coding=utf-8
#!/usr/bin/python
from __future__ import print_function
import time
import sys
import requests
from thread import *
import threading
from pyspark import SparkContext
from pyspark.streaming import StreamingContext
from pyspark.streaming.kafka import KafkaUtils


def sendMsg(x):
    #time.sleep(3)
    #global Num
    try:
        requests.post("http://localhost:1111/battery",x) #通过requests.post将数据发送给MYSQL
    except:
        print("error!")

def loadExoClients(inputFile):
    clients = []
    with open(inputFile, 'r') as fin:
        lines = fin.readlines()
        for line in lines:
            client = line.strip()
            clients.append(client.strip())
        return clients
        

def Stream(KafkaTopic,ssc):
    #global Num
    print(KafkaTopic)
    StrTopic = KafkaTopic.split('-')
    Num = StrTopic[0]
    #print(Num)
    kvs1 = KafkaUtils.createStream(ssc, "localhost:2181", KafkaTopic, {KafkaTopic: 1}) #接受来自kafka的数据， "localhost:2181"为zookeper的端口号
    #kvs2 = KafkaUtils.createStream(ssc, "localhost:2181", KafkaTopic[1], {KafkaTopic[1]: 1}) #接受来自kafka的数据， "localhost:2181"为zookeper的端口号
    lines1 = kvs1.map(lambda x: eval(x[1])['battery']) #x[1]代表接受kafka中的值其他都不行
    #lines1.pprint()
    #lines2 = kvs2.map(lambda x: eval(x[1])['battery']) #x[1]代表接受kafka中的值其他都不行
    #val = lines.map(lambda k: (k['battery'])) #取字典中需要的参数组成一个tuple
    batterychange1 = lines1.window(6,3) #创建计算窗口
    #batterychange2 = lines2.window(6,6) 
    batterymax1 = batterychange1.reduce(lambda x,y : x+y)
    batteryfinal = batterymax1.map(lambda x: {"Exo_ID":Num,"battery":x})
    #topic = kvs1.map(lambda x: x[0])
    #batterymax2 = batterychange2.reduce(lambda x,y : x+y)
    batteryfinal.pprint()
    #topic.pprint()
    #batterymax2.pprint()
    batteryfinal.foreachRDD(lambda rdd: rdd.foreach(sendMsg)) #创建新的用于发送数据的RDD

def judge(ssc):
    clients = loadExoClients('topic')
    a = str(raw_input("Enter your input: ")).split(' ')
    #a = ['1','2']
    print(a)
    for x in a:
        x = int(x)       
        Stream(clients[x-1],ssc)
       
def main():
    #global Num
    #clients = loadExoClients('topic')
    #a = raw_input("Enter your input: ");
    #Topic = clients[int(a)-1]
    try:   #停止以前的SparkContext，要不然下面创建工作会失败
        sc.stop()
        ssc.stop()
    except:  
        pass
        
    sc = SparkContext(master="local[4]",appName="StreamForCloud")
    ssc = StreamingContext(sc, 1)
    ssc.checkpoint("./streamcheckpoint")  #window()需要设置checkpoint
    judge(ssc)
    #s = threading.Thread(target=Stream, args=(Topic,ssc))
    #s.start()
    #Stream(clients[0],ssc)
    #Stream(clients[1],ssc)
    #Stream(clients[1],ssc)
    ssc.start()
    #s.start()
    ssc.awaitTermination()
    
if __name__ == '__main__':
    main()
