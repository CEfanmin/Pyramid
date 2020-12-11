# -*- coding: utf-8 -*-
import urllib,urllib2
import sys
import json

reload(sys)
sys.setdefaultencoding('utf-8')

API_KEY = '63d53b8a3bfa45c99269d299a83e0d60'
raw_TULINURL = "http://www.tuling123.com/openapi/api?key=%s&info=" % API_KEY
global queryStr

def result(N):
    TULINURL = "%s%s" % (raw_TULINURL,urllib2.quote(N))
    req = urllib2.Request(url=TULINURL)
    result = urllib2.urlopen(req).read()
    hjson=json.loads(result)
    length=len(hjson.keys())
    print(type(result))
    print(type(req))
    content=result
    if length==3:
        return 'robots:' +content+hjson['url']
    elif length==2:
        return 'robots:' +content
 
      
if __name__=='__main__':
    print "你好，请输入内容： ".decode('utf-8')
    queryStr = raw_input("我： ".decode('utf-8'))
    contents = result(queryStr)
    LAS=queryStr
    print contents
    while True:
        queryStr = raw_input("我： ".decode('utf-8'))
        sign=contents.find('？')
        if sign != -1:
           LAS=LAS+queryStr
        else:
           c=''
           i=0
           LAS = queryStr
        contents = result(LAS)
        print contents

