/*
 * Author: fanmin
 * Date: 2016/07/05
 * Function: 麦克风的软件抽象
*/
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Float32.h"
#include <alsa/asoundlib.h>
#include <pthread.h>

#include "qisr.h"
#include "msp_errors.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include "hori_micphone.h"
#include "wav_header.h"
#include "client/hori_centreClient.h"


#define	BUFFER_SIZE	4096
#define FRAME_LEN	640
#define HINTS_SIZE  100
hori_centreClient *cC;
const char *device = "default";	// 声卡设备名称
short Buffer[8*1024];    		//采样得到的buffer大小
int Buffer_size = sizeof(Buffer)>>1; //每次移动sizeof()function is １byte　来计算buffer真实的大小

/*
ID	32 	接收中心＂暂停＂消息
ID	34 	接收中心＂继续＂消息
ID	33 	发给中心＂已暂停＂消息
ID	35 	发给中心＂正常运行＂消息
*/

using namespace std;

int run_iat(const char* audio_file, const char* session_begin_params, char*out)
{
	const char*		session_id					=	NULL;
	char			rec_result[BUFFER_SIZE]		=	{'\0'};
	char			hints[HINTS_SIZE]			=	{'\0'}; //hints为结束本次会话的原因描述，由用户自定义
	unsigned int	total_len					=	0;
	int				aud_stat					=	MSP_AUDIO_SAMPLE_CONTINUE ;		//音频状态
	int				ep_stat						=	MSP_EP_LOOKING_FOR_SPEECH;		//端点检测
	int				rec_stat					=	MSP_REC_STATUS_SUCCESS ;			//识别状态
	int				errcode						=	MSP_SUCCESS ;

	FILE*			f_pcm						=	NULL;
	char*			p_pcm						=	NULL;
	long			pcm_count					=	0;
	long			pcm_size					=	0;
	long			read_size					=	0;

	if (NULL == audio_file)
		goto iat_exit;

	f_pcm = fopen(audio_file, "rb");
	if (NULL == f_pcm)
	{
		printf("\nopen [%s] failed! \n", audio_file);
		goto iat_exit;
	}

	fseek(f_pcm, 0, 2);
	pcm_size = ftell(f_pcm); //获取音频文件大小
	fseek(f_pcm, 0, 0);

	p_pcm = (char *)malloc(pcm_size);
	if (NULL == p_pcm)
	{
		printf("\nout of memory! \n");
		goto iat_exit;
	}
	read_size = fread((void *)p_pcm, 1, pcm_size, f_pcm); //读取音频文件内容
	if (read_size != pcm_size)
	{
		printf("\nread [%s] error!\n", audio_file);
		goto iat_exit;
	}
	printf("\n开始语音听写 ...\n");
	session_id = QISRSessionBegin(NULL, session_begin_params, &errcode); //听写不需要语法，第一个参数为NULL
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRSessionBegin failed! error code:%d\n", errcode);
		goto iat_exit;
	}
	while (1)
	{
		unsigned int len = 10 * FRAME_LEN; // 每次写入200ms音频(16k，16bit)：1帧音频20ms，10帧=200ms。16k采样率的16位音频，一帧的大小为640Byte
		int ret = 0;

		if (pcm_size < 2 * len)
			len = pcm_size;
		if (len <= 0)
			break;

		aud_stat = MSP_AUDIO_SAMPLE_CONTINUE;
		if (0 == pcm_count)
			aud_stat = MSP_AUDIO_SAMPLE_FIRST;

		printf(">");
		ret = QISRAudioWrite(session_id, (const void *)&p_pcm[pcm_count], len, aud_stat, &ep_stat, &rec_stat);
		if (MSP_SUCCESS != ret)
		{
			printf("\nQISRAudioWrite failed! error code:%d\n", ret);
			goto iat_exit;
		}

		pcm_count += (long)len;
		pcm_size  -= (long)len;

		if (MSP_REC_STATUS_SUCCESS == rec_stat) //已经有部分听写结果
		{
			const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
			if (MSP_SUCCESS != errcode)
			{
				printf("\nQISRGetResult failed! error code: %d\n", errcode);
				goto iat_exit;
			}
			if (NULL != rslt)
			{
				unsigned int rslt_len = strlen(rslt);
				total_len += rslt_len;
				if (total_len >= BUFFER_SIZE)
				{
					printf("\nno enough buffer for rec_result !\n");
					goto iat_exit;
				}
				strncat(rec_result, rslt, rslt_len);
			}
		}

		if (MSP_EP_AFTER_SPEECH == ep_stat)
		{
			break;
		}
		usleep(200*1000); //模拟人说话时间间隙,200ms对应10帧的音频
	}
	errcode = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
	if (MSP_SUCCESS != errcode)
	{
		printf("\nQISRAudioWrite failed! error code:%d \n", errcode);
		goto iat_exit;
	}

	while (MSP_REC_STATUS_COMPLETE != rec_stat)
	{
		const char *rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
		if (MSP_SUCCESS != errcode)
		{
			printf("\nQISRGetResult failed, error code: %d\n", errcode);
			goto iat_exit;
		}
		if (NULL != rslt)
		{
			unsigned int rslt_len = strlen(rslt);
			total_len += rslt_len;
			if (total_len >= BUFFER_SIZE)
			{
				printf("\n no enough buffer for rec_result !\n");
				goto iat_exit;
			}
			strncat(rec_result, rslt, rslt_len);
		}
		usleep(150*1000); //防止频繁占用CPU
	}
	printf("\n语音听写结束!\n");
	printf("==========================================================\n");
	printf("%s\n",rec_result);  //rec_result里面是输出的文本信息
	printf("==========================================================\n");
	memcpy(out, rec_result ,BUFFER_SIZE);

iat_exit:
	if (NULL != f_pcm)
	{
		fclose(f_pcm);
		f_pcm = NULL;
	}
	if (NULL != p_pcm)
	{	free(p_pcm);
		p_pcm = NULL;
	}
	QISRSessionEnd(session_id, hints);

}

bool wakeupFlg = false;
bool recognitionFlg = false;
void chatterCallback(const std_msgs::String::ConstPtr& msge)
{
	if (msge->data == "wkd")
	{
		printf("\nrecord and give tatget position or start chatting...\n");
		wakeupFlg = true;
		recognitionFlg = false;
	}
}

bool centercommand = true;
void eventCall(int eve,void *p)
{
	ROS_INFO("get callback [%d]",eve);
	if (eve == 32)
	{
		ROS_INFO("pause");
		centercommand = false;
		cC->pubToCenter(33);
	}
	if (eve == 34)
	{
		ROS_INFO("continue");
		centercommand = true;
		cC->pubToCenter(35);
	}
}


int main(int argc, char **argv)
{
	ros::init(argc, argv, "hori_micphone_node");
	ros::NodeHandle n;

	ros::Subscriber sub = n.subscribe("/wakeup_done",1, chatterCallback);
	ros::Publisher chatter_pub1 = n.advertise<std_msgs::String>("/wkd_feedback", 1);
	ros::Publisher chatter_pub2 = n.advertise<std_msgs::String>("/micphone_result", 10);

	ros::Rate loop_rate(10);

	system("gnome-terminal -x bash -c 'rosrun hori_voicesplit hori_voicesplit_node'");
	system("gnome-terminal -x bash -c 'rosrun hori_voicewake hori_voicewake_node'");

	int  ret = MSP_SUCCESS;
    char OUTX[BUFFER_SIZE]			=	{'\0'};
	int count_flag = 0;
	const char* login_params = "appid = 563f0251, work_dir = ."; // 登录参数，appid与msc库绑定,请勿随意改动
  	const char* session_begin_params = "sub = iat, domain = iat, language = zh_ch, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = utf8";
  	ret = MSPLogin(NULL, NULL, login_params); //第一个参数是用户名，第二个参数是密码，均传NULL即可，第三个参数是登录参数

  	std_msgs::String msgee;
  	std_msgs::String msge;

  	cC = new hori_centreClient("hori_micphone",&n,eventCall,NULL);
  	cC->pubToCenter(35);

  	while (ros::ok())
  	{
  		if(wakeupFlg && (!recognitionFlg) && centercommand)
		{
//  			std::string path1 = getenv("HOME");
//  			path1 += "/Music/record_voice.wav";
//  			ret = run_iat(path1.c_str(), session_begin_params,OUTX);
			ret = run_iat("/home/fanmin/Music/record_voice.wav", session_begin_params,OUTX);	//OUTX中存放的是char[]型数据rec_result里的内容
			if (ret == 0)
			{
				system("mv -f /home/fanmin/Music/record_voice.wav /home/fanmin/Temp/record_voice.wav");   //避免重复播放某一段音频
			}

			msge.data = OUTX;
			chatter_pub2.publish(msge);

			count_flag = 1;     //标志位
			memset(OUTX, 0, BUFFER_SIZE);   //数组OUTX清零
			wakeupFlg = false;
			recognitionFlg = true;
			msgee.data = "wkdfb";     	   //"wkd"代表wakeupdone唤醒成功
			while (count_flag!=0)		 	   //对话成功，给唤醒反馈信号
			{
				chatter_pub1.publish(msgee);
				--count_flag;
			}

		}

  		loop_rate.sleep();
  		ros::spinOnce();   //进入chatterback函数,同时接下来执行while(ros::ok())
  	}
}
