from numpy import *
import numpy as np
from mysqlTest import loadSQLData
# from mysqlTest import visualizationData
import operator
import socket
import sys
from thread import *


def classify0(inX, dataSet, labels, k):  # k is parameter
    dataSetSize = dataSet.shape[0]
    diffMat = tile(inX, (dataSetSize,1)) - dataSet
    sqDiffMat = diffMat**2
    sqDistances = sqDiffMat.sum(axis=1)
    distances = sqDistances**0.5
    sortedDistIndicies = distances.argsort()
    classCount={}
    for i in range(k):
        voteIlabel = labels[sortedDistIndicies[i]]
        classCount[voteIlabel] = classCount.get(voteIlabel,0) + 1
    sortedClassCount = sorted(classCount.iteritems(), key=operator.itemgetter(1), reverse=True)
    return sortedClassCount[0][0]

def autoNorm(dataSet):
    minVals = dataSet.min(0)
    maxVals = dataSet.max(0)
    ranges = maxVals - minVals
    normDataSet = zeros(shape(dataSet))
    m = dataSet.shape[0]
    normDataSet = dataSet - tile(minVals, (m, 1))
    normDataSet = normDataSet/tile(ranges, (m, 1))
    return normDataSet, ranges, minVals


def datingClassTest():
    hoRatio = 0.05   # parameter
    SQLDataMat, SQLDataLabels = loadSQLData()
    normMat, ranges, minVals = autoNorm(SQLDataMat)
    m = normMat.shape[0]
    numTestVecs = int(m * hoRatio)
    errorCount = 0.0
    for i in range(numTestVecs):
        classifierResult = classify0(normMat[i, :], normMat[numTestVecs:m, :], SQLDataLabels[numTestVecs:m], 3)
        print "the classifier came back with: %d, the real answer is: %d" % (classifierResult, SQLDataLabels[i])
        if (classifierResult != SQLDataLabels[i]): errorCount += 1.0
    print "the total error rate is: %f" % (errorCount / float(numTestVecs))
    print "errorCount is:",errorCount

def classifyPerson(personAge, personHeight, personWeight):
    resultList=['1.0,0.5,3.0', '0.7,0.3,3.0', '0.5,0.2,1.0']
    # personAge = float(raw_input("please enter the person age is: "))
    # personHeight = float(raw_input("please enter the person height is: "))
    # personWeight = float(raw_input("please enter the person weight is: "))
    SQLDataMat, SQLDataLabels = loadSQLData()
    # print "SQLDataMat is:", SQLDataMat
    # print "SQLDataLabels is:", SQLDataLabels

    # visualizationData()
    normMat, ranges, minVals = autoNorm(SQLDataMat)
    inArr = np.array([personAge, personHeight, personWeight])
    classifierResult = classify0((inArr-minVals)/ranges, normMat, SQLDataLabels, 3)
    # print "the recommended step states is:", resultList[classifierResult -1]
    return resultList[classifierResult -1]

# datingClassTest()
# classifyPerson(23, 175, 65)
def recoveryServer():
    HOST = '0.0.0.0'  # all IP can connect
    PORT = 8888
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print 'Socket created'

    try:
        s.bind((HOST, PORT))
    except socket.error, msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        sys.exit()
    print "Socket bind complete"

    s.listen(10)  # 10 clients
    print 'Socket now listening'

    def clientThread(conn, clientaddress):
        while True:
            data = conn.recv(1024)
            if not data:
                break
            reply = 'server reply:\t' + data
            print(clientaddress + '\t' + 'say:' + str(data))
            Data = data.split(',')
            recommended = classifyPerson(float(Data[0]),float(Data[1]),float(Data[2]))
            # print Data[0]
            # print Data[1]
            # print Data[2]
            print  "the recommended step states is:", recommended
            conn.sendall(recommended)
        conn.close()

    while 1:
        conn, addr = s.accept()
        clientaddress = addr[0] + ':' + str(addr[1])
        print 'Connected with ' + clientaddress
        start_new_thread(clientThread, (conn, clientaddress))
    s.close()

recoveryServer()


