#ifndef MachineRunningThread_H
#define MachineRunningThread_H

#include "../ICoffeeMakerHandler.h"
#include "Helper.h"
#include "opencv/cv.h"

using namespace cv;

namespace MachineRunningThread{

	// Execution function for the machinerunning thread 
	void exec(ICoffeeMakerHandler& handler){
		bool machinerunning = false;

		Mat frame_top = handler.getTopFrame();

		// Convert to HSV
		Mat HSVImage = Helper::convertToHSV(frame_top);
		// Filter image to maintain specific color range
		Mat detectColor_top = Helper::filterColor(HSVImage,  Scalar(109, 250, 90), Scalar(119, 256, 256));
		medianBlur(detectColor_top, detectColor_top, 3);

		Mat houghImage_top;
		Mat cannyImage;
		Canny(detectColor_top, cannyImage, 50, 200, 3);
		vector<vector<Point> > contours;
		cvtColor( detectColor_top, houghImage_top, CV_GRAY2BGR );
		findContours( cannyImage, contours, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE );
		int minArea = 40;
		Point minp;
		for( int i = 0; i< contours.size(); i++ )
		 {
			Moments moment = moments(contours[i]);
			double area = moment.m00;
			int posX = moment.m10/area;
			int posY = moment.m01/area;
			if( area < minArea){
				minArea = area;
				Point minp = Point(posX,posY);
				circle(houghImage_top,  Point(posX,posY), 5, Scalar(255,0,0),2);
			}
		 }
		if (minArea < 40 && minArea > 4)
			machinerunning = true;
		
		// Return result to handler class
		handler.MachineRunningThreadEnded(machinerunning, houghImage_top);
	}
}

#endif
