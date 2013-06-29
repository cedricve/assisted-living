#include "Logger.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"

#include "CoffeeMakerHandler.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[]){
	Logger::setVerbose(false);

	// Checks the command line arguments. The correct usage is: koffiedetection param1 param2 [param3]
	// param1: The path to the TOP camera
	// param2: The path to the SIDE camera. This can be a view from the left or right
	// param3: [OPTIONAL] The path to a second SIDE camera. This improves the stability of the detection algorithm. It is implied that when using a LEFT view for param2, param3 will contain a RIGHT view and vice versa.
	if(argc == 1 || argc == 2){
		cout << endl << "!!! Incorrect usage:\n" << "Usage: koffiedetection" << " " << "param1 param2 [param3]" << endl;
		cout << "\tparam1: " << "The path to the TOP camera" << endl;
		cout << "\tparam2: " << "The path to the SIDE camera. This can be a view from the left or right" << endl;
		cout << "\tparam3: [OPTIONAL]" << "The path to a second SIDE camera. This improves the stability of the detection algorithm. It is implied that when using a LEFT view for param2, param3 will contain a RIGHT view and vice versa." << endl;

		return 1;
	}

	const char* cam_top;
	const char* cam_side1;
	const char* cam_side2;
	if(argc==3){ // Only two camera's provided, so only one side camera is used.
		cam_top= argv[1];
		cam_side1 = argv[2];
		cam_side2 = 0;
	} else if(argc==4){ // All three camera's are provided, so there are two side camera's.
		cam_top= argv[1];
		cam_side1 = argv[2];
		cam_side2 = argv[3];
	}

	VideoCapture v_top, v_side1, v_side2;

	// Open the camera sources
	v_top.open(cam_top);
	v_side1.open(cam_side1);
	if(cam_side2 != 0){
		v_side2.open(cam_side2);
	}

	// Exit when not all camera's can be opened (e.g. wrong path to file) otherwise initiate the detection program
	if(!v_top.isOpened() || !v_side1.isOpened() || (cam_side2 != 0 && !v_side2.isOpened()) )
	{
		Logger::e("Unable to open all video streams..., please check the filename and try again.");

		return 1;
	} else{ 
		try{
			// The CoffeeMakerHandler takes care of all the detection algorithms and will automatically exit at the
			// end of the camere input.
			CoffeeMakerHandler handler(&v_top, &v_side1, (cam_side2 != 0 )? &v_side2 : 0);
			if(handler.initialize()){
				handler.run();
			} else {
				Logger::e("Unable to initialize the handler!");
			}
		}catch(std::exception& ex){
			std::string error = ex.what();
			Logger::e("!!! An exception was thrown: " + error);
			cout << error << endl;
		}

		destroyAllWindows();

		return 0;
	}
}
