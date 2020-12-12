import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import MySQLdb

def loadSQLData():
    try:
        '''user_basic data'''
        mysql_cn= MySQLdb.connect(host='localhost', port=3306,user='root', passwd='root', db='robot_data')
        user_df = pd.read_sql('select * from user_basic_information;', con=mysql_cn)
        user_basicLists = ['age','height','weight',]
        dataMats =[]
        for basicList in user_basicLists:
            dataMat = list(user_df[basicList])
            dataMats.append(dataMat)
        dataMats= np.array(dataMats)

        DataMat = []
        for num in range(0,dataMats.shape[1]):
            DataMat.append(list(dataMats.T[num]))

        '''label data'''
        use_df = pd.read_sql('select * from use_record;', con=mysql_cn)
        # use_recordLists= ['steplong','stepheight','stepspeed']
        use_recordLists = ['train_time']
        labelMats = []
        for use_recordList in use_recordLists:
            labelMat = list(use_df[use_recordList])
            labelMats.extend(labelMat)
        return np.array(DataMat[0:200:2]), labelMats[0:200:2]
        mysql_cn.close()

    except MySQLdb.Error:
        print "Mysql Error"

def visualizationData():
    '''visualization the data'''
    dataArr, labelArr = loadSQLData()
    print labelArr
    dataArr = np.array(dataArr)
    print "dataArr size is :", dataArr.shape[0]
    print "labelArr size is :",np.array(labelArr).shape[0]

    fig = plt.figure()
    ax = fig.add_subplot(311)
    plt.xlabel('age')
    plt.ylabel('height')
    plt.title('age and height')
    ax.scatter(dataArr[:,0],dataArr[:,1],15.0*np.array(labelArr),15.0*np.array(labelArr))
    # plt.show()

    ax = fig.add_subplot(312)
    plt.xlabel('age')
    plt.ylabel('weight')
    plt.title('age and weight')
    ax.scatter(dataArr[:,0],dataArr[:,2],15.0*np.array(labelArr),15.0*np.array(labelArr))
    # plt.show()

    ax = fig.add_subplot(313)
    plt.xlabel('height')
    plt.ylabel('weight')
    plt.title('height and weight')
    ax.scatter(dataArr[:,1],dataArr[:,2],15.0*np.array(labelArr),15.0*np.array(labelArr))
    plt.show()

visualizationData()