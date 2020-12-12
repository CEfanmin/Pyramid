#include "ros/ros.h"
#include "std_msgs/String.h"

int main(int argc, char **argv)
{
  ros::init(argc, argv, "talker");
  ros::NodeHandle n;
  ros::Publisher chatter_pub = n.advertise<std_msgs::String>("/text_result", 1);

  ros::Rate loop_rate(100);

  while (ros::ok())
  {
    std_msgs::String msg;
    std::string str("你好吗");
    msg.data = str;

    std::cout<<msg.data<<std::endl;
    chatter_pub.publish(msg);
    ros::spinOnce();
    loop_rate.sleep();

  }
  return 0;
}

