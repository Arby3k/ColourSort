#pragma once

#include "CBase4618.h"

class CBlueberrySort : public CBase4618
{
private:
	cv::VideoCapture vid;
	cv::Mat blue, hsv, check;
public:
	CBlueberrySort(cv::VideoCapture, cv::Mat, cv::Mat, cv::Mat);
	~CBlueberrySort();
	void update();
	void draw();
	void delay(double msec);
	int processImage();
	bool btnPressedPass(int pass);
	bool btnPressedFail(int fail);
	bool btnPressedSS(int ss);
	int kbhit(void);
};
