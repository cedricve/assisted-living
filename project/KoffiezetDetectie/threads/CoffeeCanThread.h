#ifndef CoffeeCanThread_H
#define CoffeeCanThread_H

#include "../ICoffeeMakerHandler.h"
#include "opencv/cv.h"
#include "Helper.h"

using namespace cv;

namespace CoffeeCanThread{
	// Static helper class to be used during the CoffeeCanThread execution
	class CoffeeCanThreadHelper{
	public:
		// Detect the coffee can inside the current frame
		static bool hasCoffeeCan(const Mat& frame, Mat &houghImage){
			bool found = false;
			Mat cannyImage;
			Canny(frame, cannyImage, 50, 200, 3);
			vector<vector<Point> > contours;
			cvtColor( frame, houghImage, CV_GRAY2BGR );
			// Detect the edges
			findContours( cannyImage, contours, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE );

			// Save the two biggest area's and the point representing the center of these area's
			Point p1,p2;
			// Area's smaller then 10 are ignored
			double area1=10, area2=10;

			// Draw each contour and detect it's orientation (horizontal/vertical) and
			// store the middle of the biggest vertical/horizontal area
			for( int i = 0; i< contours.size(); i++ )
			{
				bool horizontal = true;
				int maxx = 0, maxy =0;
				int minx = frame.cols , miny =frame.rows;
				for(int j=0; j<contours[i].size();j++){
					if( contours[i][j].x > maxx )
						maxx = contours[i][j].x;
					if( contours[i][j].y > maxy )
						maxy = contours[i][j].y;
					if( contours[i][j].x < minx )
						minx = contours[i][j].x;
					if( contours[i][j].y < miny )
						miny = contours[i][j].y;
				}
				
				if( maxy-miny > (maxx - minx)*1.5)
					horizontal = false;

				vector<Point> rect;
				rect.push_back(Point(minx, maxy));
				rect.push_back(Point(maxx, maxy));
				rect.push_back(Point(maxx, miny));
				rect.push_back(Point(minx, miny));

				rectangle(houghImage, Point(minx, miny), Point(maxx, maxy), Scalar(0,0,255), 2);
				Moments moment = moments(rect);
				double area = moment.m00;
				int posX = moment.m10/area;
				int posY = moment.m01/area;
				if( area > area1 && horizontal){
					p1= Point(posX,posY);
					area1 = area;
				}
				else if (area > area2 && ! horizontal){
					p2= Point(posX,posY);
					area2 = area;
				}
			}
			circle(houghImage,  p1, 5, Scalar(255,0,0),2);
			circle(houghImage,  p2, 5, Scalar(0,255,0),2);

			// Detect the position of the coffee can
			double distanceX;
			if(p1.x > p2.x)
				distanceX = p1.x - p2.x;
			else
				distanceX = p2.x - p1.x;

			double distanceY;
			if(p1.y > p2.y)
				distanceY = p1.y - p2.y;
			else
				distanceY = p2.y - p1.y;

			// If the coffee can position is between certain boundaries, the coffee can is "inside" of the machine
			if(p1.x >0 && p2.x >0 && distanceX <200 && distanceX >20 && distanceY < 100){
				found = true;
			}
			
			return found;
		}

		static bool handleSide(const Mat& frame,  Mat& result_frame){
			// Convert image to HSB
			Mat HSVImage = Helper::convertToHSV(frame);
			// Filter out the detection color for the coffee can
			Mat detectColor_side = Helper::filterColor(HSVImage,  Scalar(60, 170, 16), Scalar(73, 256, 256));

			// Using median blur, noise inside the frame is reduced and beter results are found
			medianBlur(detectColor_side, detectColor_side, 5);

			return hasCoffeeCan(detectColor_side, result_frame);
		}
	};

	// CoffeeCanThread execution
	void exec(ICoffeeMakerHandler& handler){
		int count = handler.getSideFrameCount();

		bool hascoffeecan_side1 = false;
		bool hascoffeecan_side2 = false;

		Mat houghImage_side1;
		Mat houghImage_side2;

		if(count >= 1){
			hascoffeecan_side1 = CoffeeCanThreadHelper::handleSide(handler.getSideFrame(false), houghImage_side1);
		}

		if(count == 2){
			hascoffeecan_side2 = CoffeeCanThreadHelper::handleSide(handler.getSideFrame(true), houghImage_side2);
		}

		// Return result to CoffeeMakerHandler
		if(count == 1){
			handler.CoffeeCanThreadEnded(hascoffeecan_side1, houghImage_side1);
		} else {
			handler.CoffeeCanThreadEnded(hascoffeecan_side1 ||hascoffeecan_side2, houghImage_side1, houghImage_side2);
		}
	}
}

#endif
