#ifndef ICoffeeMakerHandler_H
#define ICoffeeMakerHandler_H

#include "opencv/cv.h";

using namespace cv;

// This interface defines some functions implemented in the CoffeeMakerHandler class. 
class ICoffeeMakerHandler{
public: 
	virtual void CoffeeCanThreadEnded(bool hascoffeecan, Mat side_cam) = 0;
	virtual void CoffeeCanThreadEnded(bool hascoffeecan, Mat left_cam, Mat right_cam) = 0;
	virtual void CoffeeFilterHolderThreadEnded(bool hascoffeefilterholder, Mat top_cam) = 0;
	virtual void CoffeeFilterThreadEnded(bool hascoffeefilter, Mat top_cam) = 0;
	virtual void CoffeeThreadEnded(bool hascoffee, Mat top_cam) = 0;
	virtual void MachineRunningThreadEnded(bool machinerunning, Mat top_cam) = 0;
	virtual void MachineOnThreadEnded(bool machineon, Mat top_cam) = 0;
	virtual void ReservoirOpenedThreadEnded(bool reservoiropen, Mat top_cam) = 0;
	virtual void WaterThreadEnded(bool haswater, Mat side_cam) = 0;
	virtual void WaterThreadEnded(bool haswater, Mat left_cam, Mat right_cam) = 0;
	virtual Mat getTopFrame() = 0;
	virtual int getSideFrameCount() = 0;
	virtual Mat getSideFrame(bool second = false) = 0;
	virtual CoffeeMakerPosition getPosition()=0;
};

#endif