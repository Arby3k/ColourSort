#include "CSort.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <vector>
#include <pigpio.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "opencv.hpp"

std::vector<cv::Point> contour;
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;

CBlueberrySort::CBlueberrySort(cv::VideoCapture _vid, cv::Mat _blue, cv::Mat _hsv, cv::Mat _check)
{
	vid = _vid;
	blue = _blue;
	hsv = _hsv;
	check = _check;
	vid.open(0);
	if (gpioInitialise() < 0)
	{
		return ;
	}

	//BALL FLOW SERVO
	gpioSetMode(2, PI_OUTPUT);
	gpioServo(2, 2000);

	//COLOR SORTING SERVO
	gpioSetMode(26, PI_OUTPUT);

	//RED LED
	gpioSetMode(3, PI_OUTPUT);
	//GREEN LED
	gpioSetMode(4, PI_OUTPUT);
	//BLUE LED is ORANGE WIRE
	gpioSetMode(5, PI_OUTPUT);

	//BLUE WIRE is JOYSTICK
	gpioSetMode(19, PI_INPUT);
	//YELLOW WIRE is TOP BUTTON
	gpioSetMode(13, PI_INPUT);
	//PURPLE WIRE is BOTTOM BUTTON
	gpioSetMode(6, PI_INPUT);
}

CBlueberrySort::~CBlueberrySort() {
    gpioTerminate();
}

void CBlueberrySort::update()
{
    static int blueberryCount = 0;
    const int GATE_DOWN = 2000;
    const int GATE_UP = 1500;
    char keyPress = 'z';
    vid >> blue;

    if(kbhit())
           keyPress = getchar();

    if((keyPress == 's') || btnPressedSS(19))
    {
        keyPress = 'z';

        do{

        cv::waitKey(1);

        if(kbhit())
           keyPress = getchar();

        vid >> blue;

        gpioWrite(5, 1);
        int state = processImage();

        if (state == 2)
        {
            gpioServo(2, GATE_DOWN);
            gpioWrite(4, 0);
            gpioWrite(3, 0);
        }
        else
        {
            vid.release();
            if (state == 0)
            {
                blueberryCount++;
                gpioWrite(4, 1);
                gpioWrite(3, 0);

                gpioServo(2, GATE_UP);
                delay(200);
                gpioServo(2, GATE_DOWN);
                delay(150);
                gpioServo(26, 1400);
                delay(150);
                gpioServo(26, 1650);
                gpioWrite(4, 0);
            }
            else
            {
                gpioWrite(4, 0);
                gpioWrite(3, 1);

                gpioServo(2, GATE_UP);
                delay(200);
                gpioServo(2, GATE_DOWN);
                delay(150);
                gpioServo(26, 2000);
                delay(200);
                gpioServo(26, 1650);
                gpioWrite(3, 0);
            }
            vid.open(0);
        }

        cv::putText(blue, "Blueberry Count: " + std::to_string(blueberryCount), cv::Size(30, 40), CV_FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2, 1, 0);
        cv::imshow("colors", blue);

        if(btnPressedSS(19))
            break;

        if(keyPress == 'c')
        {
            blueberryCount = 0;
            keyPress = 'z';
        }

        }while(keyPress != 's');
        keyPress = 'z';
	}
	else
	{
        gpioWrite(5, 0);

        if((btnPressedPass(13)) || keyPress == 'p')
        {
            blueberryCount++;
            gpioWrite(4, 1);
            gpioWrite(3, 0);

            gpioServo(2, GATE_UP);
            delay(200);
            gpioServo(2, GATE_DOWN);
            delay(150);
            gpioServo(26, 1400);
            delay(150);
            gpioServo(26, 1650);
            gpioWrite(4, 0);
            keyPress = 'z';
        }
        else if((btnPressedFail(6)) || keyPress == 'r')
        {
            gpioWrite(4, 0);
            gpioWrite(3, 1);

            gpioServo(2, GATE_UP);
            delay(200);
            gpioServo(2, GATE_DOWN);
            delay(150);
            gpioServo(26, 2000);
            delay(200);
            gpioServo(26, 1650);
            gpioWrite(3, 0);
            keyPress = 'z';
        }
        if(keyPress == 'c')
        {
            blueberryCount = 0;
            keyPress = 'z';
        }
	}
	cv::putText(blue, "Blueberry Count: " + std::to_string(blueberryCount), cv::Size(30, 40), CV_FONT_HERSHEY_PLAIN, 2, cv::Scalar(255, 0, 0), 2, 1, 0);
	cv::imshow("colors", blue);
}

void CBlueberrySort::draw()
{

}
void CBlueberrySort::delay(double msec) // got from stackoverflow
{
	double elapsed_time;
	double freq = cv::getTickFrequency(); // Get tick frequency

	double start_tic = cv::getTickCount(); // Get number of ticks since event (such as computer on)
	double curr_tic;

	do
	{
        curr_tic = cv::getTickCount();
        elapsed_time = (curr_tic - start_tic) / freq * 1000;
	} while ((int)elapsed_time < msec);
}


int CBlueberrySort::processImage()
{
	cv::cvtColor(blue, hsv, CV_BGR2HSV);

	static cv::Mat gray;
	static cv::Mat thr;

	cv::cvtColor(blue, gray, CV_RGB2GRAY);
	cv::threshold(gray, thr, 100, 255, cv::THRESH_BINARY);
	cv::erode(thr, thr, cv::Mat(), cv::Point(-1, -1), 5);
    cv::dilate(thr, thr, cv::Mat(), cv::Point(-1, -1), 5);

	cv::inRange(hsv, cv::Scalar(0, 0, 50), cv::Scalar(180, 255, 255), check);
	findContours(thr, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    if (contours.size() == 0)
        return 2;

    cv::inRange(hsv, cv::Scalar(100, 150, 0), cv::Scalar(120, 255, 255), check);

    cv::erode(check, check, cv::Mat(), cv::Point(-1, -1), 5);
    cv::dilate(check, check, cv::Mat(), cv::Point(-1, -1), 5);

    findContours(check, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < contours.size(); i++)
    {
        cv::Rect r = boundingRect(contours.at(i));

        if (r.area() > 50000)
        {
            return 0;
        }
		drawContours(check, contours, i, cv::Scalar(255, 255, 255), CV_FILLED, 8, hierarchy);
    }
    return 1;
}


bool CBlueberrySort::btnPressedPass(int pass)
{
	int data;
	static int timerCount = 1;
	static bool statusChanged = false;

	data = gpioRead(pass);

	if (data == 0)
	{
		if ((timerCount == 0) && (!statusChanged))
		{
			statusChanged = true;
			return true;
		}
		else
		{
			timerCount--;
		}
	}
	else
	{
		timerCount = 1;
		statusChanged = false;
	}
	return false;
}

bool CBlueberrySort::btnPressedFail(int fail)
{
	int data;
	static int timerCount = 1;
	static bool statusChanged = false;

	data = gpioRead(fail);

	if (data == 0)
	{
		if ((timerCount == 0) && (!statusChanged))
		{
			statusChanged = true;
			return true;
		}
		else
		{
			timerCount--;
		}
	}
	else
	{
		timerCount = 1;
		statusChanged = false;
	}
	return false;
}

bool CBlueberrySort::btnPressedSS(int ss)
{
	int data;
	static int timerCount = 1;
	static bool statusChanged = false;

	data = gpioRead(ss);

	if (data == 0)
	{
		if ((timerCount == 0) && (!statusChanged))
		{
			statusChanged = true;
			return true;
		}
		else
		{
			timerCount--;
		}
	}
	else
	{
		timerCount = 1;
		statusChanged = false;
	}
	return false;
}

/*////////////////////////////////////////////////////////////////////////////////////////////////
/
/This function was taken from https://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
/It is used to implement kbhit on linux
/
////////////////////////////////////////////////////////////////////////////////////////////////*/


int CBlueberrySort::kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}
