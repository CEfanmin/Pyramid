#include "hori_centreClient.h"
#include <memory.h>
#include <stdlib.h>

hori_centreClient *phori_centreClient;
void centreFeedbackCallback(const centre::event::ConstPtr& msg){
    //ROS_INFO("I heard [%d]",msg->eventID);
    if(phori_centreClient->isfeedback(msg->eventID)){
        phori_centreClient->feedBackCome(msg->eventID);
    }
}

void *centreClientThread(void *arg){
    ROS_INFO("[cC][%s] client run",((hori_centreClient *)arg)->name.c_str());
    ((hori_centreClient *)arg)->run();
    return NULL;
}

hori_centreClient::hori_centreClient(std::string nodeName,ros::NodeHandle *nodeHandle,void (* CallBackFun)(int eventID,void *p),void *p):name(nodeName),n(nodeHandle),param(p){
    loadCfgFile();
    tocentrePub = n->advertise<centre::event>("tocentre", 1000);
    centreFeedbackSub = n->subscribe("centreFeedback", 1000, centreFeedbackCallback);
    loopRate = new ros::Rate(1);
    phori_centreClient = this;
    isSetCallBack = false;
    currentState = 0;
    feedBackState = 0;
    if(CallBackFun != NULL){
        pCallBackFun = CallBackFun;
        isSetCallBack = true;
    }
        int error = pthread_create(&pthread,NULL,centreClientThread,(void *)this);
        if(error!=0)
        {
            ROS_INFO("centreClient thread is not created ... ");
            return;
        }
}

hori_centreClient::~hori_centreClient(){
    delete [] isFeedBack;
}

void hori_centreClient::loadCfgFile(void){
    std::string path = getenv("HOME");
    path += "/centreCfg/centreCfg.json";
    loadCfg load(path);
    size_t arraySize = load.getMaxID();
    isFeedBack = new bool[arraySize];
    memset(isFeedBack,0,arraySize * sizeof(bool));
    load.getClientParam(name,isFeedBack);
}

void hori_centreClient::run(void){
    centre::event msg;
    for(;;){
        loopRate->sleep();
        //ROS_INFO("start loop msg value ");
        msg.eventID = currentState;
        //ROS_INFO("start loop pub ");
        tocentrePub.publish(msg);
        //ROS_INFO("finish loop pub ");
    }
}

bool hori_centreClient::isfeedback(int eventID) const{
    return isFeedBack[eventID];
}

void hori_centreClient::feedBackCome(int eventID){
    if(feedBackState == eventID)return;
    feedBackState = eventID;
    if(isSetCallBack){
        (*pCallBackFun)(eventID,param);
    }
}

void hori_centreClient::pubToCenter(int eventID){
    currentState = eventID;
    centre::event msg;
    msg.eventID = eventID;
    tocentrePub.publish(msg);
}

int hori_centreClient::checkFeedBack(void) const{
    return feedBackState;
}
