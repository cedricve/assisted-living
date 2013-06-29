#ifndef CoffeeFilterThread_H
#define CoffeeFilterThread_H

#include "../ICoffeeMakerHandler.h"
#include "Helper.h"
#include "opencv/cv.h"

using namespace cv;

namespace CoffeeFilterThread {

	// Execution function for the CoffeeFilterThread, to detect a filter inside the coffeefilter holder
	void exec(ICoffeeMakerHandler& handler){
		bool hascoffeefilter = false;

		Mat frame_top = handler.getTopFrame();
		CoffeeMakerPosition pos = handler.getPosition();
		
		// When the coffeeholder has been found, this thread will be executed.
		// The region of the coffeeholder will be analysed, and a decision will be made: 
		// a) coffeefilter? 
		// b) coffee? 
		// c) or coffeefilter + coffee??
		//
		// This function can be found in threads/Helper.h and only uses the TOP-CAMERA
		Mat result = Helper::validateCoffeeOrFilter(hascoffeefilter,frame_top,false, pos);

		// Return result to coffeemaker handler
		handler.CoffeeFilterThreadEnded(hascoffeefilter, result);
	}
}

#endif
