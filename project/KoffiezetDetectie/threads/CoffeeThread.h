#ifndef CoffeeThread_H
#define CoffeeThread_H

#include "../ICoffeeMakerHandler.h"
#include "Helper.h"

using namespace cv;

namespace CoffeeThread{

	// Execution function for the coffee thread. This thread will run only when the coffeefilter holder
	// is outside of the machine. The thread will return a value indicating if coffee is found inside the holder.
	void exec(ICoffeeMakerHandler& handler){
		bool hascoffee = false;

		Mat frame_top = handler.getTopFrame();
		CoffeeMakerPosition pos = handler.getPosition();
		
		// When the coffeeholder has been found, this thread will be executed.
		// The region of the coffeeholder will be analysed, and a decision will be made: 
		// a) coffeefilter? 
		// b) coffee? 
		// c) or coffeefilter + coffee??
		//
		// This function can be found in threads/Helper.h and only uses the TOP-CAMERA
		Mat result = Helper::validateCoffeeOrFilter(hascoffee,frame_top,true, pos);
		// Return result to coffeemaker handler
		handler.CoffeeThreadEnded(hascoffee, result);
	}
}

#endif
