#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <getopt.h>

#include "ros/ros.h"
#include "std_msgs/String.h"

#include "qisr.h"
#include "msp_cmn.h"
#include "msp_errors.h"

#define MAX_PATH	(260)
#define APPID		("563f0251")
#define RATE		(16000)
#define CHNS		(2)
#define BITS		(16)
#define FRMSMP		(320)
#define BLKCNT		(16)
#define BLKLEN		(FRMSMP*CHNS*BITS/8)
#define DEVICE 		("default")

ros::Publisher *micpt = NULL;

unsigned char buf[BLKCNT][BLKLEN];	// audio buffer
char load_path[MAX_PATH] = {0};		// audio file path for load
char save_path[MAX_PATH] = {0};		// audio file path for save
char dev_name[MAX_PATH] = {DEVICE};	// device name
char iat_flag = 1;					// enable or disable iat process
unsigned int rate = RATE;			// sample rate
int chns = CHNS;					// channels
int bits = BITS;					// bit width
int loop = 100;						// loop count
volatile int readp = 0;				// read position
volatile int writep = 0;			// write position
volatile int run_flag = 0;			// run flag
char appid[9] = {APPID};			// appid
int bvep  = 3000;					// time for vad begin endpoint detection
int evep  = 1000;					// time for vad end endpoint detection
int farmode = 1;					// far or normal engine used
sem_t sem_audio;					// semaphore for audio data
pthread_t id_iat = 0;				// thread iat process
pthread_t id_key = 0;				// thread key process
pthread_t id_rec = 0;				// thread rec process

int snd_pcm_init(snd_pcm_t *handle)
{
	int rc = 0;
	int dir = 0;
	snd_pcm_hw_params_t *hwparams = NULL;

	// allocate the snd_pcm_hw_params_t structure on the stack
	snd_pcm_hw_params_alloca(&hwparams);

	// fill it in with default values
	if ((rc = snd_pcm_hw_params_any(handle, hwparams)) < 0) {
		printf("%s : %s\n", "snd_pcm_hw_params_any", snd_strerror(rc));
		return rc;
	}

	// set acccess type, this can be either SND_PCM_ACCESS_RW_INTERLEAVED or SND_PCM_ACCESS_RW_NONINTERLAEAVED
	if ((rc = snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("3.%s : %s\n", "snd_pcm_hw_params_set_access", snd_strerror(rc));
		return rc;
	}

	// set pcm data format (default SND_PCM_FORMAT_S16_LE, signed 16-bit little-endian format)
	if ((rc = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16_LE)) < 0) {
		printf("%s : %s\n", "snd_pcm_hw_params_set_format", snd_strerror(rc));
		return rc;
	}

	// set channels, one channel(mono), or two channels (stereo)
	if ((rc = snd_pcm_hw_params_set_channels(handle, hwparams, chns)) < 0) {
		printf("%s : %s\n", "snd_pcm_hw_params_set_channels", snd_strerror(rc));
		return rc;
	}

	// set sample rate.
	if ((rc = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rate, &dir)) < 0) {
		printf("%s : %s\n", "snd_pcm_hw_params_set_rate_near", snd_strerror(rc));
		return rc;
	}
	if (0 != dir) {
		printf("rate not supported!, should set to near rate(%d)!\n", rate);
		return rc;
	}

	// set period size(in frams) to fragments
	unsigned int period_time = 0;
	snd_pcm_uframes_t period_size = FRMSMP;
	if ((rc = snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, &dir)) < 0) {
		printf("%s : %s\n", "snd_pcm_hw_params_set_period_size_near", snd_strerror(rc));
		return rc;
	}
	if (0 != dir) {
		printf("period size not supported! should set to near size(%d)!\n", (int)period_size);
		return rc;
	}
	if ((rc = snd_pcm_hw_params_get_period_size(hwparams, &period_size, &dir)) < 0) {
		printf("x.%s : %s\n", "snd_pcm_hw_params_get_period_size", snd_strerror(rc));
		return rc;
	}
	if ((rc = snd_pcm_hw_params_get_period_time(hwparams, &period_time, &dir)) < 0) {
		printf("x.%s : %s\n", "snd_pcm_hw_params_get_period_time", snd_strerror(rc));
		return rc;
	}
	// set number of period_count. period_count used to be called as fragments
	unsigned int period_count = BLKCNT;
	if ((rc = snd_pcm_hw_params_set_periods_near(handle, hwparams, &period_count, &dir)) < 0) {
		printf("8.%s : %s\n", "snd_pcm_hw_params_set_periods_near", snd_strerror(rc));
		return rc;
	}
	if (0 != dir) {
		printf("periods count not supported!, should set to near count(%d)!\n", period_count);
		return rc;
	}
	if ((rc = snd_pcm_hw_params_get_periods(hwparams, &period_count, &dir)) < 0) {
		printf("x.%s : %s\n", "snd_pcm_hw_params_get_periods", snd_strerror(rc));
		return rc;
	}

	snd_pcm_uframes_t buffer_size = period_count * period_size;

	if ((rc = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size)) < 0) {
		printf("x.%s : %s\n", "snd_pcm_hw_params_get_buffer_size", snd_strerror(rc));
		return rc;
	}

	// apply hw parameters to the device
	if ((rc = snd_pcm_hw_params(handle, hwparams)) < 0)
	{
		printf("0.%s : %s\n", "snd_pcm_hw_params", snd_strerror(rc));
		return rc;
	}
	rc = 0;
	return rc;
}

void *thread_rec_proc(void *args)
{
	int rc = 0;
	int fd = 0;
	snd_pcm_t *handle = NULL;

	// audio file or device(sound card) open
	if (strlen(load_path)) {
		// open audio file for load
		if ((fd = open(load_path, O_RDONLY)) < 0) {
			printf("\nfile[%s] open[%d]!\n", load_path, fd);
			goto error;
		}
	}
	else {
		// open pcm device for recording(capture).
		// the last parameter of this function is the mode. if this is set to 0, the standard mode is used.
		// other possible values are SND_PCM_NONBLOCK and SND_PCM_ASYNC.
		// if SND_PCM_NONBLOCK is used, read/write access to the pcm device will return immediately.
		// if SND_PCM_ASYNC is specified, SIGIO will be emitted whenever a period has been completely processed by the soundcard.
		if ((rc = snd_pcm_open(&handle, dev_name, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
			printf("%s : %s\n", "snd_pcm_open", snd_strerror(rc));
			goto error;
		}

		// config device(sound card) with parameters
		if ((rc = snd_pcm_init(handle)) < 0)
			goto error;
	}

	// audio data load process
	while (run_flag)
		if (fd > 0) {
			// load audio from file
			if ((rc = read(fd, buf[writep], BLKLEN)) <= 0)
				break;
			writep = (writep + 1) % BLKCNT;
			sem_post(&sem_audio);
			usleep(BLKLEN / (chns * rate * bits));
		}
		else {
			rc = snd_pcm_readi(handle, buf[writep], FRMSMP);
			if (!run_flag)
				break;
			// read data from hardware
			switch (rc) {
			case -EBADFD:
				printf("%s!\n", snd_strerror(rc));
				break;
			case -ESTRPIPE:
				printf("%s!\n", snd_strerror(rc));
				break;
			case -EPIPE:	// EPIPE means overrun
				printf("%s!\n", snd_strerror(rc));
				rc = snd_pcm_prepare(handle);
				printf("%s : %s\n", "snd_pcm_prepare", snd_strerror(rc));
				break;
			case FRMSMP:
				writep = (writep + 1) % BLKCNT;
				if (writep == readp)
					printf("\ndata loss!\n");
				sem_post(&sem_audio);
				break;
			default:
				printf("\nunknown[%d]!\n", rc);
				fflush(stdout);
				break;
			}
		}
	// close audio file or device
	if (fd > 0)
		close(fd);
	else {
		// close pcm device
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	}
	rc = MSP_SUCCESS;

error:
	run_flag = 0;
	return NULL;
}

std_msgs::String msg;
int iat_proc_func(int fd)
{
	int rc = MSP_SUCCESS;
	int ep_state = MSP_EP_LOOKING_FOR_SPEECH;
	int rec_state = MSP_REC_STATUS_SUCCESS;
	int aud_state = MSP_AUDIO_SAMPLE_FIRST;
	const char *sess_id = NULL;
	char sess_param[MAX_PATH] = {0};
	sprintf(sess_param, "%s,sub=iat,ptt=1,sample_rate=%d,vad_bos=%d,vad_eos=%d,result_type=plain,domain=fariat,result_encoding=utf8",
			farmode ? "aue=speex-wb;10" : "aue=raw", rate, bvep, evep);

	// session begin
	sess_id = QISRSessionBegin(NULL, sess_param, &rc);
	if (MSP_SUCCESS != rc) {
		printf("%s error[%d]!\n", "QISRSessionBegin", rc);
		goto error;
	}

	while (run_flag) {
		// wait for audio data arrive
		sem_wait(&sem_audio);

		// convert stereo to mono(left channel)
		if (2 == chns)
			for (rc = 1; rc < FRMSMP; rc++) {
				buf[readp][rc*2+0] = buf[readp][rc*4+0];
				buf[readp][rc*2+1] = buf[readp][rc*4+1];
			}

		// save audio data to file
		if (fd > 0 && write(fd, buf[readp], BLKLEN/chns) != BLKLEN/chns)
			printf("file write error!\n");

		// run iat process
		// send audio data to server
		if ((rc = QISRAudioWrite(sess_id, (const void *)buf[readp], BLKLEN/chns, aud_state, &ep_state, &rec_state)) != MSP_SUCCESS) {
			printf("%s error[%d]!\n", "QISRAudioWrite", rc);
			goto error;
		}
		aud_state = MSP_AUDIO_SAMPLE_CONTINUE;

		// partial result ready
		if (MSP_REC_STATUS_SUCCESS == rec_state) {
			// recvice result from server
			const char *rslt = QISRGetResult(sess_id, &rec_state, 0, &rc);
			if (MSP_SUCCESS != rc) {
				printf("%s error[%d]!\n", "nQISRGetResult", rc);
				goto error;
			}
			if (NULL != rslt) {
				printf("来自火星的数据:　　%s\n", rslt);
				msg.data = rslt;
				if (msg.data != ""){
					micpt->publish(msg);
				}
				fflush(stdout);
			}
		}
		// voice end detected
		if (MSP_EP_AFTER_SPEECH == ep_state)
			break;
		readp = (readp + 1) % BLKCNT;
	}

	if (rec_state) {
		// wait for left result, notify server audio over
		if ((rc = QISRAudioWrite(sess_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &rec_state, &rec_state)) != MSP_SUCCESS) {
			printf("%s error[%d]!\n", "QISRAudioWrite", rc);
			goto error;
		}

		// wait for result
		while (MSP_REC_STATUS_COMPLETE != rec_state) {
			// recvice result from server
			const char *rslt = QISRGetResult(sess_id, &rec_state, 0, &rc);
			if (MSP_SUCCESS != rc) {
				printf("%s error[%d]!\n", "nQISRGetResult", rc);
				goto error;
			}
			if (NULL != rslt) {
				//printf("\33[4;31m%s\33[0m ", rslt);	// underline("\33[4m"), red("\33[31m"), default("\33[0m")
				fflush(stdout);
			}
			// sleep a while
			usleep(1000);
		}
		printf("\n");
	}

	// close session
	if (iat_flag && (rc = QISRSessionEnd(sess_id, NULL)) != MSP_SUCCESS) {
		printf("%s error[%d]!\n", "QISRSessionEnd", rc);
		goto error;
	}
	rc = MSP_SUCCESS;

error:
	return rc;
}

void *thread_iat_proc(void *args)
{
	int rc = 0;
	const char* save_path = "./AudioRecord.wav";
	// open audio file for save
	int fd = 0;
	if (strlen(save_path)) {
		// Open
		if ((fd = open(save_path, O_RDWR|O_CREAT|O_APPEND|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)) < 0) {
			printf("\nfile[%s] open[%d]!\n", save_path, fd);
			goto error;
		}
	}

	if (iat_flag) {
		// login
		char login_param[32] = {0};
		sprintf(login_param, "appid=%s,work_dir=.", appid);
		if ((rc = MSPLogin(NULL, NULL, login_param)) != MSP_SUCCESS) {
			printf("MSPLogin failed[%d]!\n", rc);
			goto error;
		}

		// audio data process
		printf("来，说一句(press enter to exit).....\n");
		while (run_flag && loop--)
			if ((rc = iat_proc_func(fd)) != MSP_SUCCESS)
				goto error;
		// logout
		if ((rc = MSPLogout()) != MSP_SUCCESS) {
			printf("MSPLogout failed[%d]!\n", rc);
			goto error;
		}
	}
	else if (fd > 0)
		while (run_flag) {
			sem_wait(&sem_audio);

			// convert stereo to mono(left channel)
			if (2 == chns)
				for (rc = 1; rc < FRMSMP; rc++) {
					buf[readp][rc*2+0] = buf[readp][rc*4+0];
					buf[readp][rc*2+1] = buf[readp][rc*4+1];
				}

			// save audio data to file
			if ((rc = write(fd, buf[readp], BLKLEN/chns)) != BLKLEN/chns)
				printf("file write error[%d]!\n", rc);
		}

	// close audio file
	if (fd > 0)
		close(fd);
	rc = MSP_SUCCESS;
error:
	run_flag = 0;
	return NULL;
}

void *thread_key_proc(void *args)
{
	// any key to stop recording
	struct timeval time = {0, 100*1000}; // 100ms
	int fd = open ("/dev/tty", O_RDONLY | O_NONBLOCK);
	int ret = 0;

	while (run_flag) {
		fd_set rfs;
		FD_ZERO(&rfs);
		FD_SET(fd, &rfs);
		ret = select(fd + 1, &rfs, NULL, NULL, &time);
		if (ret < 0) {
			printf("\nselect error[%d]!\n", ret);
			exit(0);
		}
		if (0 == ret)	// timeout
			continue;
		if (FD_ISSET(fd, &rfs))	// key pressed
			break;
	}

	close(fd);

	// disable flag
	run_flag = 0;
	sem_post(&sem_audio);
	sem_post(&sem_audio);
	return NULL;
}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "xunfeimic");
	ros::NodeHandle n;
	ros::Publisher chatter_pub = n.advertise<std_msgs::String>("/micphone_result", 1);
	micpt = &chatter_pub;

	memset(load_path, 0, MAX_PATH);
	memset(save_path, 0, MAX_PATH);
	int rc = 0;

	// init semaphore
	sem_init(&sem_audio, 0, 0);

	// enable flag
	run_flag = 1;

	// creat threads
	rc = pthread_create(&id_rec, NULL, thread_rec_proc, NULL);
	rc = pthread_create(&id_iat, NULL, thread_iat_proc, NULL);
	rc = pthread_create(&id_iat, NULL, thread_key_proc, NULL);

	// wait threads exit
	rc = pthread_join(id_iat, NULL);
	rc = pthread_join(id_rec, NULL);
	rc = pthread_join(id_key, NULL);

	// destory semaphore
	sem_destroy(&sem_audio);
	return 0;
}
