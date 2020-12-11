#include "ros/ros.h"
#include "loadCfg.h"


loadCfg::loadCfg(std::string cfgfilename){
    std::ifstream myfile(cfgfilename.c_str());
    std::string filestr,tmp;
	while (! myfile.eof() ) {
        myfile>>tmp;
        filestr += tmp;
	}
	//ROS_INFO("load file str:[%s]",filestr.c_str());
	pJsonParser = new Json::Reader(Json::Features::strictMode());
	if (!pJsonParser->parse(filestr, root))
	{
	    ROS_INFO("[loadCfg][ERROR]Read json failed");
	}
}

loadCfg::~loadCfg(){
    delete pJsonParser;
}

int loadCfg::getMaxID(){
    return root["maxID"].asInt();
}

void loadCfg::getParam(int* ID2nodeArray,
                       std::vector<std::string>& rsclist,
                       std::vector<std::string>& nodeList,
                       nodeBehavior* nodeBehaviorArray)
{
    const Json::Value resourceList = root["resourceList"];
    for(size_t i = 0;i < resourceList.size();i ++){
        rsclist.push_back(resourceList[i]["name"].asString());
    }
    const Json::Value nodelist = root["nodeList"];
    for(size_t i = 0;i < nodelist.size();i ++){
        nodeList.push_back(nodelist[i]["nodeName"].asString());
        const Json::Value nodeBehav = nodelist[i]["nodeBehavior"];
        for(size_t j = 0;j < nodeBehav.size();j ++){
            int currentID = nodeBehav[j]["ID"].asInt();
            nodeBehaviorArray[currentID].action = nodeBehav[j]["action"].asInt();
            nodeBehaviorArray[currentID].param1 = nodeBehav[j]["p1"].asInt();
            nodeBehaviorArray[currentID].param2 = nodeBehav[j]["p2"].asInt();
            nodeBehaviorArray[currentID].param3 = nodeBehav[j]["p3"].asInt();
            ID2nodeArray[currentID] = i;
        }
    }
}

void loadCfg::getClientParam(std::string nodeName, bool* paramList){
    const Json::Value nodelist = root["nodeList"];
    for(size_t i = 0;i < nodelist.size();i ++){
        if(nodeName == nodelist[i]["nodeName"].asString()){
            ROS_INFO("[cC]got node param");
            const Json::Value nodeBehav = nodelist[i]["nodeBehavior"];
            for(size_t j = 0;j < nodeBehav.size();j ++){
                int currentID = nodeBehav[j]["ID"].asInt();
                paramList[currentID] = true;
            }
        }
    }
}

