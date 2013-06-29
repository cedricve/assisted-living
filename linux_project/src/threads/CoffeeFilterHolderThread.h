#ifndef CoffeeFilterHolderThread_H
#define CoffeeFilterHolderThread_H

#include "../ICoffeeMakerHandler.h"
#include "Helper.h"
#include "opencv/cv.h"

using namespace cv;

namespace CoffeeFilterHolderThread {

	// The following variabled are used during execution of the thread, to remember
	// the last position of the coffee filter holder. When the holder is moved outside
	// of the view of the camera, this would insinuate that the holder is back inside of
	// the machine. By checking the last position (and calculation the direction where 
	// the holder was/is moving) this is detected.
	static Vec3f latest_holder_position(0,1000,0);
	static Vec3f previous_holder_position(0,1000,0);
	static int counter = 0;
	static bool in_position = false;

	// The execution function of the coffeefilterholder thread
	void exec(ICoffeeMakerHandler& handler){
		bool hascoffeefilterholder = false;

		Mat frame_top = handler.getTopFrame();
		CoffeeMakerPosition pos = handler.getPosition();
		
		// Find the coffeefilter holder
		Mat result;
		Vec3f holder = Helper::findCoffeeHolder(frame_top,result,pos); 

		// When holder is found, the thread returns true. When not found, the last position of the 
		// holder is checked to see if the holder is inside of the machine or outside the view of the
		// camera.
		previous_holder_position = latest_holder_position;
		if(holder[2] > 0){
			latest_holder_position = holder;
			hascoffeefilterholder = true;
			counter = 0;
			in_position = false;
		}

		if(latest_holder_position[1] != 1000 && previous_holder_position==latest_holder_position)
			counter++;

		if(latest_holder_position[1] != 1000){
			hascoffeefilterholder = true;
			if(counter > 5){		
				if(!in_position){
					Mat look_position = Helper::crop(frame_top, Rect(Point(0,pos.getY()-pos.getRatio()*100-100),Point(frame_top.cols,pos.getY()-pos.getRatio()*100)));
					look_position = Helper::convertToHSV(look_position);
					look_position = Helper::filterColor(look_position, Scalar(0,208,166),Scalar(98,256,256));
					medianBlur(look_position,look_position,5);

					vector<vector<Point> > contours;
					findContours( look_position, contours, CV_RETR_LIST , CV_CHAIN_APPROX_NONE );
					if(contours.size()>0){
						hascoffeefilterholder = false;
						in_position = true;
					}

				}
				else
					hascoffeefilterholder = false;
			}
		}

		// Return result to coffeemaker handler
		handler.CoffeeFilterHolderThreadEnded(hascoffeefilterholder, result);
	}
}

#endif
