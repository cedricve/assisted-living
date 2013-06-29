#ifndef ReservoirOpenedThread_H
#define ReservoirOpenedThread_H

#include "../ICoffeeMakerHandler.h"
#include "opencv/cv.h"
#include "Helper.h"

using namespace cv;

class CoffeeMakerHandler;
namespace ReservoirOpenedThread{
	// Helper class for the reservoiropened thread
	class ReservoirOpenedThreadHelper{
	public:
		// Detects if the water reservoir is opened or not
		static bool hasWaterReservoir(const Mat &detectColor, Mat &houghImage){
			bool found = true;
			Mat cannyImage;
			Canny(detectColor, cannyImage, 50, 200, 3);
			vector<vector<Point> > contours;
			cvtColor( detectColor, houghImage, CV_GRAY2BGR );
			findContours( cannyImage, contours, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE );
			int totalArea = 0;
			for( int i = 0; i< contours.size(); i++ )
			{
				int maxx = 0, maxy =0;
				int minx = detectColor.cols , miny =detectColor.rows;
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

				vector<Point> rect;
				rect.push_back(Point(minx, maxy));
				rect.push_back(Point(maxx, maxy));
				rect.push_back(Point(maxx, miny));
				rect.push_back(Point(minx, miny));
				double areaRechthoek = (maxx - minx) * (maxy-miny);
				totalArea += areaRechthoek;
				rectangle(houghImage, Point(minx, miny), Point(maxx, maxy), Scalar(0,0,255), 2);
				Moments moment = moments(rect);
				double area = moment.m00;
				int posX = moment.m10/area;
				int posY = moment.m01/area;
				circle(houghImage,  Point(posX,posY), 5, Scalar(255,0,0),2);
			}
			if (totalArea >= 1000) 
				found = false;
			return found;
		}
	};

	// Execution function for the reservoiropened thread
	void exec(ICoffeeMakerHandler& handler){
		Mat frame_top = handler.getTopFrame();
		CoffeeMakerPosition pos = handler.getPosition();
		
		// Return HSV image
		Mat HSVImage = Helper::convertToHSV(frame_top);

		// Filter image to maintain only specific color range
		Mat detectColor_top = Helper::filterColor(HSVImage, Scalar(0,208,166),Scalar(98,256,256));
		detectColor_top = Helper::crop(detectColor_top, Rect(Point(pos.getX()-80,pos.getY()+50),Point(pos.getX()+50,pos.getY()+100)));
		
		medianBlur(detectColor_top, detectColor_top, 5);

		Mat houghImage_top;
		bool opened = ReservoirOpenedThreadHelper::hasWaterReservoir(detectColor_top, houghImage_top);
		Mat result_top;

		handler.ReservoirOpenedThreadEnded(opened,houghImage_top);
	}
}

#endif
