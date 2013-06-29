#ifndef MachineOnThread_H
#define MachineOnThread_H

#include "../ICoffeeMakerHandler.h"
#include "CoffeeMakerHandler.h"
#include "CoffeeMakerPosition.h"
#include "Helper.h"
#include "opencv/cv.h"
#include <math.h>

using namespace cv;

namespace MachineOnThread{

	// Static helper class used during the execution of the machineon thread. This 
	// thread detects the status of the on/off switch on the machine.
	class MachineOnThreadHelper{
	public:
		static bool detectButton(const Mat &frame, Mat &outputImage,ICoffeeMakerHandler& handler){
			// Crop the image to cut out the expected region of the on/off-switch
			CoffeeMakerPosition pos = handler.getPosition();
			const float  PI_F=3.14159265358979f;
			float angleInDegrees = (pos.getAngle() * 180) / PI_F;
			double w = pos.getWidth()/2;
			double h = pos.getHeight()/2;
			double angle = pos.getAngle();
			double ratio = pos.getRatio();
			double dis_x = 121*ratio;
			double dis_y = 10*ratio;

			int x = pos.getX();
			int y = pos.getY();
			float start_angle = atan(dis_y/dis_x);
			double distance = sqrt(pow(dis_x,2)+pow(dis_y,2));
			angle -= start_angle;
			x -= cos(angle)*distance;
			if(angle <0 )
				y -= sin(angle)*distance;
			else 
				y += sin(angle)*distance;

			Mat M,rotated,croppedImage,cropped;
			RotatedRect rect = RotatedRect(Point2f(x,y),Size2f(1.2*h,2*h),angleInDegrees);

			cropped = Helper::crop(frame, rect.boundingRect());
			croppedImage = cropped.clone();
			
			// Filter to maintain only specific color range
			Mat detectColor;
			detectColor = Helper::filterColor(croppedImage, Scalar(60, 100, 50), Scalar(256, 256, 256));

			// Median blur is used to reduce noise to get a better result
			medianBlur(croppedImage, croppedImage, 5);

			Mat cannyImage;
			Canny(detectColor, cannyImage, 10, 200, 5);

			// Detect contours
			vector<vector<Point> > contours;
			findContours( cannyImage, contours, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE );
			cvtColor(detectColor, outputImage, CV_GRAY2BGR);

			// Show contours
			for(int i = 0 ; i < contours.size() ; i++){
				drawContours( outputImage, contours, 0, Scalar(0, 0, 255), 1);
			}

			if(contours.size() >= 1){
				return true;
			}else{
				return false;
			}
		}

	};

	// Execution of the MachineOn thread
	void exec(ICoffeeMakerHandler& handler){	
		Mat frame_top = handler.getTopFrame();

		Mat outputImage;
		bool machineon = MachineOnThreadHelper::detectButton(frame_top, outputImage, handler);

		// Return result to handler
		handler.MachineOnThreadEnded(machineon, outputImage);
	}
}

#endif
