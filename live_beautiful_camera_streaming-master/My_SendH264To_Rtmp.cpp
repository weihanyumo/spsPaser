/**
 * 
 * ���������ڽ��ڴ��е�H.264����������RTMP��ý���������
 * This program can send local h264 stream to net server as rtmp live stream.
 */
#include "stdafx.h"
#include <stdio.h>
#include "librtmp_send264.h"
#include "live_beautiful_camera_streaming.h"


FILE *fp_send1;

////���ļ��Ļص�����
////we use this callback function to read data from buffer
//int read_buffer1(unsigned char *buf, int buf_size )
//{
//	if(!feof(fp_send1))
//	{
//
//		int true_size=fread(buf,1,buf_size,fp_send1);
//		return buf_size;
//	}
//	else
//	{
//		return -1;
//	}
//}

//�����λ������Ļص�����
//we use this callback function to read data from buffer
int read_buffer2(unsigned char *buf, int buf_size )
{
	
	
	int size = 0;
	int read_size = 0;
	
	m_pCircleBuffer->Read(buf,buf_size, (size_t *)&read_size);
	return read_size;

}

int My_SendH264To_Rtmp()
{
	//fp_send1 = fopen("result2.h264", "rb");

	//��ʼ�������ӵ�������
	RTMP264_Connect("rtmp://localhost/live/livestream");
	
	
	RTMP264_Send(read_buffer2);
	
	//�Ͽ����Ӳ��ͷ������Դ
	RTMP264_Close();

	return 0;
}

