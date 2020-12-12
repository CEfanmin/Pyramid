# -*- coding: utf-8 -*-
from numpy import *
from mysqlTest import loadSQLData
# from mysqlTest import visualizationData
import operator
import socket
from thread import *
from time import clock

# 创建kd树
from operator import itemgetter
import sys
import numpy as np

# kd-tree每个结点中主要包含的数据结构如下
class KdNode(object):
	def __init__(self, dom_elt, split, left, right):
		self.dom_elt = dom_elt  # k维向量节点(k维空间中的一个样本点)
		self.split = split  # 整数（进行分割维度的序号）
		self.left = left  # 该结点分割超平面左子空间构成的kd-tree
		self.right = right  # 该结点分割超平面右子空间构成的kd-tree

class KdTree(object):
	def __init__(self, data):
		k = len(data[0])  # 数据维度

		def CreateNode(split, data_set):  # 按第split维划分数据集exset创建KdNode
			if not data_set:  # 数据集为空
				return None
			# key参数的值为一个函数，此函数只有一个参数且返回一个值用来进行比较
			# operator模块提供的itemgetter函数用于获取对象的哪些维的数据，参数为需要获取的数据在对象中的序号
			# data_set.sort(key=itemgetter(split)) # 按要进行分割的那一维数据排序
			data_set.sort(key=lambda x: x[split])
			split_pos = len(data_set) // 2  # //为Python中的整数除法
			median = data_set[split_pos]  # 中位数分割点
			split_next = (split + 1) % k  # cycle coordinates

			# 递归的创建kd树
			return KdNode(median, split,
			              CreateNode(split_next, data_set[:split_pos]),  # 创建左子树
			              CreateNode(split_next, data_set[split_pos + 1:]))  # 创建右子树
		self.root = CreateNode(0, data)  # 从第0维分量开始构建kd树,返回根节点

# KDTree的前序遍历进行验证构建的树
def preorder(root):
	print (root.dom_elt)
	if root.left:  # 节点不为空
		preorder(root.left)
	if root.right:
		preorder(root.right)

# 搜索kd树
from math import sqrt
from collections import namedtuple
# 定义一个namedtuple,分别存放最近坐标点、最近距离和访问过的节点数
result = namedtuple("Result_tuple", "nearest_point  nearest_dist  nodes_visited")
Distance = []
def find_nearest(tree, point):
	k = len(point)  # 数据维度
	def travel(kd_node, target, max_dist):
		if kd_node is None:
			return result([0] * k, float("inf"), 0)  # python中用float("inf")和float("-inf")表示正负无穷

		nodes_visited = 1
		s = kd_node.split  # 进行分割的维度
		pivot = kd_node.dom_elt  # 进行分割的“轴”

		if target[s] <= pivot[s]:  # 如果目标点第s维小于分割轴的对应值(目标离左子树更近)
			nearer_node = kd_node.left  # 下一个访问节点为左子树根节点
			further_node = kd_node.right  # 同时记录下右子树
		else:  # 目标离右子树更近
			nearer_node = kd_node.right  # 下一个访问节点为右子树根节点
			further_node = kd_node.left

		temp1 = travel(nearer_node, target, max_dist)  # 进行遍历找到包含目标点的区域
		nearest = temp1.nearest_point  # 以此叶结点作为“当前最近点”

		dist = temp1.nearest_dist  # 更新最近距离
		Distance.append(dist)
		nodes_visited += temp1.nodes_visited

		if dist < max_dist:
			max_dist = dist  # 最近点将在以目标点为球心，max_dist为半径的超球体内

		temp_dist = abs(pivot[s] - target[s])  # 第s维上目标点与分割超平面的距离
		if max_dist < temp_dist:  # 判断超球体是否与超平面相交
			return result(nearest, dist, nodes_visited)  # 不相交则可以直接返回，不用继续判断

		# ----------------------------------------------------------------------
		# 计算目标点与分割点的欧氏距离
		temp_dist = sqrt(sum((p1 - p2) ** 2 for p1, p2 in zip(pivot, target)))

		if temp_dist < dist:  # 如果“更近”
			nearest = pivot   # 更新最近点
			dist = temp_dist  # 更新最近距离
			max_dist = dist   # 更新超球体半径

		# 检查另一个子结点对应的区域是否有更近的点
		temp2 = travel(further_node, target, max_dist)

		nodes_visited += temp2.nodes_visited
		if temp2.nearest_dist < dist:  # 如果另一个子结点内存在更近距离
			nearest = temp2.nearest_point  # 更新最近点
			dist = temp2.nearest_dist  # 更新最近距离
		return result(nearest, dist, nodes_visited)

	return travel(tree.root, point, float("inf"))  # 从根节点开始递归

def classify0(inX, dataSet, labels, k):  # k is parameter
    t0 = clock()
    listdataSet = list(dataSet)
    kd_tree = KdTree(listdataSet)
    ret2 = find_nearest(kd_tree, list(inX))  # N个样本点中寻找离目标最近的点
    #print (ret2)
    sortedDistIndicies = np.array(Distance).argsort()
    # print (sortedDistIndicies)
    t1 = clock()
    #print ('search time is:', t0-t1,'s')
    classCount={}
    for i in range(k):
        voteIlabel = labels[sortedDistIndicies[i]]
        classCount[voteIlabel] = classCount.get(voteIlabel,0) + 1
    # print (classCount)
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

def stepClassTest():
    hoRatio = 0.10   # parameter
    SQLDataMat, SQLDataLabels = loadSQLData()
    normMat, ranges, minVals = autoNorm(SQLDataMat)
    m = normMat.shape[0]
    numTestVecs = int(m * hoRatio)
    errorCount = 0.0
    for i in range(numTestVecs):
        classifierResult = classify0(normMat[i, :], normMat[numTestVecs:m, :], SQLDataLabels[numTestVecs:m], 3)
        print ("the classifier came back with: %d, the real answer is: %d" % (classifierResult, SQLDataLabels[i]))
        if (classifierResult != SQLDataLabels[i]): errorCount += 1.0
    print ("the total error rate is: %f" % (errorCount / float(numTestVecs)))
    print ("errorCount is:",errorCount)

def classifyPerson(personAge, personHeight, personWeight):
    resultList=['1.0,0.5,3.0', '0.7,0.3,3.0', '0.5,0.2,1.0']
    SQLDataMat, SQLDataLabels = loadSQLData()
    # visualizationData()
    normMat, ranges, minVals = autoNorm(SQLDataMat)
    inArr = np.array([personAge, personHeight, personWeight])
    classifierResult = classify0((inArr-minVals)/ranges, normMat, SQLDataLabels, 3)
    # print "the recommended step states is:", resultList[classifierResult -1]
    return resultList[classifierResult -1]

# stepClassTest()

def recoveryServer():
    HOST = '0.0.0.0'  # all IP can connect
    PORT = 8891
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print ('Socket created')

    try:
        s.bind((HOST, PORT))
    except socket.error, msg:
        print ('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])
        sys.exit()
    print ("Socket bind complete")

    s.listen(10)  # 10 clients
    print ('Socket now listening')

    def clientThread(conn, clientaddress):
        while True:
            data = conn.recv(1024)
            if not data:
                break
            reply = 'server reply:\t' + data
            print(clientaddress + '\t' + 'say:' + str(data))
            Data = data.split(',')
            recommended = classifyPerson(float(Data[0]),float(Data[1]),float(Data[2]))
            print  ("the recommended step states is:", recommended)
            conn.sendall(recommended)
        conn.close()

    while 1:
        conn, addr = s.accept()
        clientaddress = addr[0] + ':' + str(addr[1])
        print ('Connected with ' + clientaddress)
        start_new_thread(clientThread, (conn, clientaddress))
    s.close()

recoveryServer()


