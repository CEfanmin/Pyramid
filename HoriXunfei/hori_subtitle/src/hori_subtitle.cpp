#include "ros/ros.h"
#include "std_msgs/String.h"
#include "client/readJson.h"

loadText Text;
string OUTX;
ros::Publisher *p = NULL;
void loadText::load_cfgText(string &input, string& output)
{
	Json::Reader reader;
  	Json::Value root;
  	input+=".json";
	ifstream myfile(input.c_str(), ios::binary);
	std_msgs::String msg;

	if( !myfile.is_open() )
	{
	    cout << "Error opening file\n";
	}

    std::string filestr,tmp;
    while (!myfile.eof() )
    {
        myfile >> tmp;
        filestr += tmp;
	}

  	if (reader.parse(filestr, root))
	{
    	const Json::Value arrayObj = root["details"];
    	for (size_t i=0; i<arrayObj.size()+1;i++)
		{
     	    int difference = arrayObj[i+1]["time"]. asInt();
			string subtitle = arrayObj[i]["detailsText"].asString();
			output = subtitle;
			msg.data = OUTX.c_str();
			if (msg.data != "")
			{
				cout<< "detailsText is:" << subtitle <<endl;
	     	    cout<< "next time is:" <<difference<<endl;
				p->publish(msg);
				if (difference!=0)
				{
					sleep(difference);
				}
			}
        }
  	}
	else
	{
    	cout << "parse error" << endl;
  	}
    myfile.close();
}

void subtitleDisplay(const std_msgs::String::ConstPtr& msg)
{
	ROS_INFO("msg data is:%s ", msg->data.c_str());
	string input = msg->data.c_str();
	std::string::size_type position = input.find("wavefile");
	if(position != input.npos)
	{
		std::string subs1 = input.substr(position+8,input.size()-8);
		Text.load_cfgText(subs1,OUTX);
	}
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "listener");
	ros::NodeHandle n;

	ros::Subscriber sub = n.subscribe("/text_result", 1, subtitleDisplay);
	ros::Publisher chatter_pub = n.advertise<std_msgs::String>("/subtitle_result", 1);
	p = &chatter_pub;

	ros::spin();
	return 0;
}


