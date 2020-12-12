#!/bin/bash
# chkconfig:2345 85 15
# Description:autoStart shell

sleep 15
cd /home/cloudCenter/messageDistribute/
python cloudCenterTest.py &

cd /home/cloudCenter/recommendStep/
python knnRec.py &

