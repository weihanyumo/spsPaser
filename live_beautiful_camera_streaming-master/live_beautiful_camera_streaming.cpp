// live_beautiful_camera_streaming.cpp : �������̨Ӧ�ó������ڵ㡣
//
/********************************************************
*
*���ߣ�shiter
*all rights reservered
*������������ϵĴ��벻һһ�г��������߱�ʾ��л��������Ϣ�����߲����и�����
*���͵�ַ��http://blog.csdn.net/wangyaninglm/article/details/51056101
*
*2016.04.07
*
****************************************************
*/




#include "stdafx.h"
#include "live_beautiful_camera_streaming.h"
#include "CircleBuffer.h"

using namespace std;
using namespace cv;
#define  GOLABLE_BUFFER_SIZE 1024*64



CPs_CircleBuffer* m_pCircleBuffer;


void CameraToH264(void *pcn) 
{

	CvCapture* capture;
	//VideoCapture capture;
	Mat frame;

	//-- 1. Load the cascades
	if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return ; };
	//if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

	VideoCapture cap(0); //��Ĭ�ϵ�����ͷ��
	if(!cap.isOpened())  //����Ƿ�򿪳ɹ�
		return ;

	int w = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int h = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	int yuv_bufLen = w*h*3/2;
	unsigned char* pYuvBuf = new unsigned char[yuv_bufLen];

	int index = 0;///

	for(;;)
	{
		Mat frame;
		cap >> frame; // ������ͷ�л�ȡ�µ�һ֡
		//detectAndDisplay( frame );
		imshow("original image", frame);
		//��esc�Ƴ�
		if(waitKey(40) == 27) break;
		//detectAndenhance(frame);
		//imshow("enhance image",equalizeIntensityHist(frame));
		Mat temp;
		//SkinRGB(&IplImage(frame),&IplImage(temp));
		//highlight(frame);
		MySkinEnhance(frame);
		//highlight(frame);
		//bilateralFilter(frame,temp,9,1.0,1.0);//�����˲�ûʲôЧ��
		//adaptiveBilateralFilter( frame, temp , Size(5,5),50, 20.0, Point(-1, -1),BORDER_DEFAULT );
		//blur(frame,temp,Size(7,7),Point(-1,-1));
		//imshow("filter image",temp);


		/////////////////////////////////////////
		cv::Mat yuvImg;
		cv::cvtColor(frame, yuvImg, CV_BGR2YUV_I420);
		memcpy(pYuvBuf, yuvImg.data, yuv_bufLen*sizeof(unsigned char));
		MyOneFrameYuvtoH264(w,h,(char *)pYuvBuf);
	
	}
	//����ͷ����VideoCapture�������������ͷ�
	waitKey(0);

	delete[] pYuvBuf;
}

void H264ToRtmp(void *pcn) 
{
	Sleep(3000);
	My_SendH264To_Rtmp();
	
}


/**
* @function main
*/int main( void )
{
	m_pCircleBuffer = new CPs_CircleBuffer(GOLABLE_BUFFER_SIZE);
	
	HANDLE h_cameraToh264;
	h_cameraToh264 = (HANDLE)_beginthread((void(__cdecl *)(void *))CameraToH264,0,(void *)1);
	
	

	HANDLE h_h264ToRtmp;
	h_h264ToRtmp = (HANDLE)_beginthread((void(__cdecl *)(void *))H264ToRtmp,0,(void *)1);


	WaitForSingleObject(h_cameraToh264,INFINITE);
	WaitForSingleObject(h_h264ToRtmp,INFINITE);
	

	   Sleep(100);
	return 0;
}
