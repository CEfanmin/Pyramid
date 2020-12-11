#pragma once
#include <string>
#include <fstream>
#include "json/json.h"

typedef struct{
    int action;
    int param1;
    int param2;
    int param3;
}nodeBehavior;

class loadCfg{
public:
    loadCfg(std::string cfgfilename);
    ~loadCfg();

private:
    Json::Reader *pJsonParser;
    Json::Value root;
public:
    int getMaxID();
    void getParam(int *ID2nodeArray,
                  std::vector<std::string> &rsclist,
                  std::vector<std::string> &nodeList,
                  nodeBehavior *nodeBehaviorArray);
    void getClientParam(std::string nodeName,bool *paramList);
};
