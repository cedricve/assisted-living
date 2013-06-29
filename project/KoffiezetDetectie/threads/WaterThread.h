#ifndef WaterThread_H
#define WaterThread_H

#include "../ICoffeeMakerHandler.h"
#include "opencv/cv.h"

using namespace cv;

namespace WaterThread{
	// Helper class for the water thread
	class WaterThreadHelper {
	public:
		static Mat handleSide(const Mat & side, bool& result){
			// Convert to HSV
			Mat HSVImage =  Helper::convertToHSV(side);

			// Filter color
			Mat detectColor = Helper::filterColor(HSVImage,  Scalar(60, 170, 16), Scalar(73, 256, 256));
			medianBlur(detectColor, detectColor, 5);

			int j = 0;
			int i = 0;
			
			// Range where ocurrence of coffeejug has to be searched (0-50) y-coordinates
			int offset = 50;
			
			// If green is spotted at the top side camera, the coffeejug could be there
			// Else only waterreservoir has been opened
			while(!result && i < side.size().width){
				while(!result && j < offset){
					Scalar intensity = side.at<uchar>(j,i);
					if(intensity[0]>0)
						result = true;
					j++;
				}
				i++;
			}
			return detectColor;
		}
	};

	// Execution function for the water thread
	void exec(ICoffeeMakerHandler& handler){
		int count = handler.getSideFrameCount();

		bool haswater_left = false;
		bool haswater_right = false;

		Mat detectColor_side1;
		Mat detectColor_side2;

		if(count >= 1){
			detectColor_side1 = WaterThreadHelper::handleSide(handler.getSideFrame(), haswater_left);
		}

		if(count == 2){
			detectColor_side2 = WaterThreadHelper::handleSide(handler.getSideFrame(true), haswater_right);
		}

		// Return value to the handler class
		if(count == 1){
			handler.WaterThreadEnded(haswater_left, detectColor_side1);
		} else {
			handler.WaterThreadEnded(haswater_left || haswater_right, detectColor_side1, detectColor_side2);
		}
	}
}

#endif
