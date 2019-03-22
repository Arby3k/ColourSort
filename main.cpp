#include <string>
#include <iostream>
#include <thread>

#include <iomanip>

#include <cmath>
#include "CSort.h"
#include <pigpio.h>

// OpenCV Include
#include <opencv2/opencv.hpp>


// OpenCV Library
#pragma comment(lib,".\\opencv\\lib\\opencv_world310d.lib")
using namespace std;

enum type { DIGITAL, ANALOG, SERVO };

int main(int argc, char* argv[]){
	cv::VideoCapture _vid;
	cv::Mat blue, _hsv, check;
	CBlueberrySort cam(_vid, blue, _hsv, check);
	cam.run();
}

