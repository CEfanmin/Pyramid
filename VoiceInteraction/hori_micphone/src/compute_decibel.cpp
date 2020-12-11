#include <math.h>

short buffer[8*1024];    		//采样得到的buffer大小
int buffer_size = sizeof(buffer)>>1;    //每次移动sizeof()function is １byte　来计算buffer真实的大小
double rms(short *buffer)
{

	long int square_sum = 0.0;
	for(int i=0; i<buffer_size; i++)
	{
		square_sum += (buffer[i] * buffer[i]);
	}
	double result = sqrt(square_sum/buffer_size);
	return result;
}
