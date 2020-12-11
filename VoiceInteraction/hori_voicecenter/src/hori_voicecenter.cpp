/*
 * Author: fanmin
 * Date: 2016/07/28
 * Function: 与中心节点结合后的语音中心节点
*/
#include <ros/ros.h>
#include <geometry_msgs/Pose.h>
#include <std_msgs/String.h>
#include <std_msgs/UInt16.h>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <pthread.h>

#include "/usr/include/python2.7/Python.h"
#include "client/hori_centreClient.h"
#include "client/voice_json.h"

#define	BUFFER_SIZE	4096

hori_centreClient *cC;

/*
ID	46 	接收中心＂暂停＂消息
ID	48 	接收中心＂继续＂消息
ID	47 	发给中心＂已暂停＂消息
ID	49 	发给中心＂正常运行＂消息
ID	50 	发给中心＂申请使用喇叭＂消息
ID	51 	发给中心＂接收中心返回许可＂消息
*/
using namespace std;
int chattinglib(char *input, char *output)
{
	std::string path = getenv("HOME");
	path+="/catkin_ws/src/hori_voicecenter/chatting.txt";
    ifstream in_file(path.c_str());

    if (!in_file)
     { cerr << "oops! unable to open input file\n"; return -1; }

	string word;
	string chat;
	while (in_file >> word)
	{
		in_file >> chat;
		if (char *str =strstr(input,word.c_str()))
		{
			cout <<"chat is:" << chat <<endl;;
			memcpy(output,chat.c_str(),chat.size());
			cC->pubToCenter(50);
			break;
		}
	}
	return 0;
}

/* ros的回调函数 */
bool wakeupFlg = false;
bool recognitionFlg = false;
char OUTX[BUFFER_SIZE]	= {'\0'};
bool centercommand = true;
void chatterCallback(const std_msgs::String::ConstPtr& msge)
{
	if (msge->data.length()!= 0 &&(centercommand == true))
	{
		printf("\ngive tatget position or start chatting...\n");
		strcpy(OUTX,msge->data.c_str()); // 将订阅到的文字赋值给OUTX
		wakeupFlg = true;
		recognitionFlg = false;
	}
}

bool apply_horn = false;
void eventCall(int eve,void *p)
{
	ROS_INFO("get callback [%d]",eve);
	if (eve == 46)
	{
		ROS_INFO("pause");
		centercommand = false;
		cC->pubToCenter(47);
	}
	if (eve == 48)
	{
		ROS_INFO("continue");
		centercommand = true;
		cC->pubToCenter(48);
	}

	if (eve ==51)
	{
		ROS_INFO("apply sucess!");
		apply_horn = true;
	}

}
void* spin_thread(void*)
{
	ros::spin();
	return NULL;
}

std::string tulingAnwser;
void tulingAPI(char* input)
{
	Py_Initialize();
	if (!Py_Initialize){
		std::cout<<"initial error" <<std::endl;
	}
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('/home/fanmin/catkin_ws/src/hori_voicecenter/src')"); //注意这里roslaunch的时候改成绝对路径
    PyObject *pModule = NULL, *pDict = NULL, *pFunc = NULL, *pArg = NULL, *output = NULL;
    pModule = PyImport_ImportModule("tuling");
    if (!pModule){
    	std::cout<<"pModule error" <<std::endl;
    }

    pDict = PyModule_GetDict(pModule);
    if (!pDict){
    	std::cout<<"pDict error" <<std::endl;
    }
    pFunc = PyDict_GetItemString(pDict, "result");
    pArg = Py_BuildValue("(s)",input);
    output = PyEval_CallObject(pFunc, pArg);
    char* anwser=NULL;
    PyArg_Parse(output, "s", &anwser);
    printf("anwser = %s\n", anwser);
    std::string::size_type position;
    std::string subs0(anwser);
   	position = subs0.find("text");
   	tulingAnwser = subs0.substr(position+6,subs0.size()-6);
   	std::cout <<"tulingAnwser is "<<tulingAnwser <<std::endl;
    Py_Finalize();
}

/* 主函数入口 */
int main(int argc, char **argv)
{
    ros::init(argc, argv, "hori_voicecenter");
    ros::NodeHandle n;

    ros::Subscriber sub = n.subscribe("/micphone_result",1, chatterCallback);

    //ros::Publisher chatter_pub = n.advertise<geometry_msgs::Pose>("/prmi/hori_control", 10);

    ros::Publisher chatter_pub1 = n.advertise<std_msgs::String>("/voice_command", 1);
    ros::Publisher chatter_pub2 = n.advertise<std_msgs::String>("/text_result", 10);

    ros::Rate loop_rate(100);


  	char pose[BUFFER_SIZE]	=	{'\0'};
  	loaddata voicepoint;

	printf("\n#######################################################\n");
	printf("##                  	语音中心执行节点            ##\n");
	printf("#######################################################\n\n");

	cC = new hori_centreClient("hori_voicecenter",&n,eventCall,NULL);
	cC->pubToCenter(49);
	if (centercommand != true)
	{
		cC->pubToCenter(47);
	}

   pthread_t ros_spin_thread;
   int error = pthread_create (&ros_spin_thread, NULL , spin_thread,NULL);


while(ros::ok())
{
	if(wakeupFlg && (!recognitionFlg) && centercommand)
	{
		geometry_msgs::Pose targetPosition;
		std_msgs::String voice_command;
		std_msgs::String msg;

		//导览点问答
	    char*outchat = new char[1000];
	    memset(outchat,0,1000*sizeof(char));
	    chattinglib(OUTX,outchat);     //检查chattinglib库
		string inputx(OUTX);
		std::cout << "input is:" << inputx << std::endl;

		//打断说话
		char *kill_voice =strstr(OUTX,"停止");
		if(kill_voice!=NULL){

			cC->pubToCenter(52);
			sleep(1);
			ROS_INFO("kill voiceCommand");
			cC->pubToCenter(53);

		}
		else{
			string text (outchat);   //outchat中存放的是chat中的内容
			cC->pubToCenter(50);
			while(!apply_horn)
			{
				loop_rate.sleep();
			}

			cC->pubToCenter(49);
			if (text.empty())
			{
				//text="warning";
				tulingAPI(OUTX);
				std::cout << "end tuling" <<std::endl;
				text = tulingAnwser;
			}
			std::cout << "text is: " << text  <<std::endl;
			msg.data = text;
			chatter_pub2.publish(msg);
			apply_horn = false;
			//导览点
			string outcommand;
			voicepoint.load_cfgdata(inputx, outcommand);
			voice_command.data = outcommand;
			std::cout << "voice command is:" << voice_command.data<<std::endl;
			if (outcommand != "")
			{
				chatter_pub1.publish(voice_command);
			}
		}

		wakeupFlg = false;
		recognitionFlg = true;
	}

	loop_rate.sleep();
}
}
