#ifndef COFFEE_HANDLER_H
#define COFFEE_HANDLER_H

#include "Logger.h"
#include <math.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <exception>
#include <thread>
#include "CoffeeMakerStatus.h"
#include "CoffeeMakerPosition.h"

#include "threads/CoffeeThread.h"
#include "threads/CoffeeCanThread.h"
#include "threads/CoffeeFilterHolderThread.h"
#include "threads/MachineRunningThread.h"
#include "threads/MachineOnThread.h"
#include "threads/ReservoirOpenedThread.h"
#include "threads/CoffeeFilterThread.h"
#include "threads/WaterThread.h"
#include <mutex>

using namespace std;
using namespace cv;

// The following constants define the names of all the windows
static const char* cam_window_name = "Camera";
static const char* cam_coffeecan_name = "CoffeeCan Thread";
static const char* cam_coffeefilterholder_name = "CoffeeFilterHolder Thread";
static const char* cam_coffee_name = "Coffee Thread";
static const char* cam_machinerunning_name = "MachineRunning Thread";
static const char* cam_machineon_name = "MachineOn Thread";
static const char* cam_reservoiropened_name = "ReservoirOpened Thread";
static const char* cam_coffeefilter_name = "CoffeeFilter Thread";
static const char* cam_water_name = "Water Thread";
static const int frame_delay = 30; // Delay between frames
static int windowwidth = 700; // Frame window width
static int windowheight = 700; // Frame window height

/*
This class is the core of the system. It runs the actual program,
starts the threads, maintains the status, shows the output, ...
*/
class CoffeeMakerHandler : public ICoffeeMakerHandler
{
public:
	CoffeeMakerHandler(VideoCapture* cam_top, VideoCapture* cam_side1, VideoCapture* cam_side2)
		: cam_top(cam_top), cam_side1(cam_side1), cam_side2(cam_side2), runningthreads(0) {
	}

	// Initialization function of the program.
	bool initialize(){
		Logger::i("COFFEEMAKER BEHAVIOUR DETECTION", false);
		Logger::i("================================", false);
		Logger::v("Handler initialization started.");

		coffeefilter_window_open = false;
		water_window_open = false;
		machinerunning_window_open = false;
		coffee_window_open = false;

		// Calibrate the program
		if(calibrate()){
			showFixedWindows();

			Logger::v("Handler initialization ended.");
			return true;
		} else {
			return false;
		}
	}

	// This function is used to calibrate the program.
	// The first frame is taken and the position of the 
	// machine is detected. 
	// The position contains besides an x and y value, also
	// the zoom factor and the rotation of the machine.
	bool calibrate(){
		Logger::v("Auto-calibration started.");
		Mat frame;
		*cam_side1 >> frame;
		if(cam_side2 != 0){
			*cam_side2 >> frame;
		}
		*cam_top >> frame; // Take all 3 frames to make sure the video sources stay in sync

		// Blur image to remove noise
		medianBlur(frame, frame, 3);
		// Make grayscale
		vector<Mat> channels;
		split(frame, channels);
		cvtColor(channels[0], frame, CV_BayerGB2GRAY);

		// Threshold frame to remove unwanted colors
		Mat grayThresh;
		threshold(frame,grayThresh,200,255,CV_THRESH_BINARY); 

		// Find contours in the remaining image
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		findContours( grayThresh, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

		Mat drawing = Mat::zeros( grayThresh.size(), CV_8UC3 );
		// Find the contour containing the calibration cross
		int found = -1;
		RotatedRect foundRect;
		float foundRatio;
		vector<Point> foundApprox;
		bool foundTop; // If the most left point is matched to the bottom corner, this variable is false. True when the top corner is matched.
		for( int i = 0; i< contours.size(); i++ )
		{
			vector<Point> contour= contours[i];
			vector<Point> approx;
			approxPolyDP(contour, approx, 5, true); // Find polygon points from contour points

			const double ratioEPS = 0.15;
			const double lowerratio = 0.21 * (1-ratioEPS);
			const double upperratio = 0.21 * (1+ratioEPS);
			// This line seems to work for each movie, because the cross has 8 contour points. Using this lines filters out the contours with too many or too little points.
			if(approx.size() == 8){ 
				// Find the box containing the polygon points. In general, the wide line of the cross connects the diagonal (left top corner and right bottom corner).
				RotatedRect rect = minAreaRect(contour);
				// Since other shapes can contain 8 points, check if this shape has the proper proportions

				bool top = rect.size.height < rect.size.width;
				double ratio = (top)? rect.size.height/rect.size.width : rect.size.width/rect.size.height;

				if( ratio > lowerratio && ratio <= upperratio){ // If the rotaded rectangle has the proper ratio, it's probably the box we are looking for.
					found = i;
					foundRect = rect;
					foundApprox = approx;
					foundRatio = ratio;
					break;
				}
			}
		}


		if(found != -1){
			const float  PI_F=3.14159265358979f;

			drawContours( drawing, contours, found, Scalar( 240,240,240 ), 2, 8, hierarchy, 0, Point() );

			// Retrieve the rotated rectangle points
			Point2f rect_points[4]; 
			foundRect.points( rect_points );

			// Calculate the position values
			int width;
			if(foundTop){
				width = sqrt(pow(foundRect.size.width, 2) + pow(foundRect.size.height, 2));
			} else {
				width = sqrt(pow(foundRect.size.height, 2) + pow(foundRect.size.width, 2));
			}

			int height = width * foundRatio;

			float dY = rect_points[3].y - rect_points[1].y;
			float dX = rect_points[3].x - rect_points[1].x;
			double angle = atan(dY / dX);
			for( int j = 0; j < 4; j++ )
				line( drawing, rect_points[j], rect_points[(j+1)%4], Scalar( 0,255,51 ), 1, 8 );

			int x = foundRect.center.x;
			int y = foundRect.center.y;
			float ratio = (float)width/200.0; 

			//circle(drawing, Point(x, y), 5, Scalar(255, 0, 0), 5);
			RNG rng(12345);
			for(int i = 0; i < 500;i++){
				Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
			}
			for( int i = 0; i< foundApprox.size(); i++ )
			{
				Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
				circle(drawing, foundApprox[i], 3, color, 3);
			}

			position = CoffeeMakerPosition(x, y, width, height, ratio, angle);
			Logger::v("Auto-calibration done.");

			//imshow("Calib", drawing);
			return true;
		} else {
			return false;
		}
	}

	// Return the position of the machine (calculated during calibration)
	virtual CoffeeMakerPosition getPosition(){
		std::lock_guard<std::mutex> lock(position_mutex);
		return position;
	}

	/*
	The following functions are implementation from the ICoffeeMakerHandler
	and are used in the threads to retrieve information from the handler class
	or return results to it.
	*/
	virtual void CoffeeCanThreadEnded(bool hascoffeecan, Mat left_cam, Mat right_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);				
		runningthreads -= 1;

		status.setHasCoffeeCanState(hascoffeecan);
		showCoffeeCanAlgo(left_cam, right_cam);
	}

	virtual void CoffeeCanThreadEnded(bool hascoffeecan, Mat side_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);				
		runningthreads -= 1;

		status.setHasCoffeeCanState(hascoffeecan);
		showCoffeeCanAlgo(side_cam);

	}

	virtual void CoffeeThreadEnded(bool hascoffee, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setHasCoffeeState(hascoffee);
		showCoffeeAlgo(top_cam);
	}

	virtual void CoffeeFilterHolderThreadEnded(bool hascoffeefilterholder, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setCoffeeFilterHolderState(hascoffeefilterholder);
		showCoffeeFilterHolderAlgo(top_cam);
	}

	virtual void CoffeeFilterThreadEnded(bool hascoffeefilter, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;
		
		status.setHasFilterState(hascoffeefilter);
		showCoffeeFilterAlgo(top_cam);
	}

	virtual void MachineRunningThreadEnded(bool machinerunning, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);				
		runningthreads -= 1;

		status.setMachineRunningState(machinerunning);
		showMachineRunningAlgo(top_cam);
	}

	virtual void MachineOnThreadEnded(bool machineon, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setMachineOnState(machineon);	
		showMachineOnAlgo(top_cam);
	}

	virtual void ReservoirOpenedThreadEnded(bool opened, Mat top_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setReservoirOpenedState(opened);
		showReservoirOpenedAlgo(top_cam);
	}

	virtual void WaterThreadEnded(bool haswater, Mat left_cam, Mat right_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setWaterState(haswater);
		showWaterAlgo(left_cam, right_cam);
	}

	virtual void WaterThreadEnded(bool haswater, Mat side_cam){
		std::lock_guard<std::mutex> lock(threadend_mutex);		
		runningthreads -= 1;

		status.setWaterState(haswater);
		showWaterAlgo(side_cam);
	}

	virtual int getSideFrameCount(){ // Returns the number of side camera's used (1 or 2)
		std::lock_guard<std::mutex> lock(sideframe_mutex);
		if(cam_side2 != 0){
			return 2;
		} else {
			return 1;
		}
	}

	virtual Mat getSideFrame(bool second){ // Gets one of the current side frames
		std::lock_guard<std::mutex> lock(sideframe_mutex);
		Mat side;

		if(second){
			currentframe_side2.copyTo(side);
		} else {
			currentframe_side1.copyTo(side);
		}

		return side;
	}

	virtual Mat getTopFrame(){ // Returns the current top frame
		std::lock_guard<std::mutex> lock(topframe_mutex);
		Mat ret;
		currentframe_top.copyTo(ret);
		return ret;
	}

	// Execute the program
	void run(){
		Logger::v("Handler running...");
		Logger::i("Press ESC to exit");

		status = CoffeeMakerStatus();
		int frameNr = 0;
		int interval = 0;
		int alarm_interval = 0;

		for(;;)
		{
			// Get the next frames
			Mat frame_top, frame_side1, frame_side2;
			*cam_top >> frame_top;
			*cam_side1 >> frame_side1;
			if(cam_side2 != 0){
				*cam_side2 >> frame_side2;
			}

			// Exit of the frames are empty
			if(frame_top.empty() || frame_side1.empty() || (cam_side2 != 0 && frame_side2.empty())){
				break;
			}

			// Convert the frames to RGB and store them inside the class
			vector<Mat> channels;
			split(frame_top, channels);
			cvtColor(channels[0], currentframe_top, CV_BayerRG2RGB);

			vector<Mat> channels2;
			split(frame_side1, channels2);
			cvtColor(channels2[0], currentframe_side1, CV_BayerRG2RGB);

			if(cam_side2 != 0){
				vector<Mat> channels3;
				split(frame_side2, channels3);
				cvtColor(channels3[0], currentframe_side2, CV_BayerRG2RGB);
			}

			if(interval > 333 && runningthreads == 0){	// EVERY THIRD OF A SECOND, START THREADS TO DETERMINE THE CURRENT 
				// STATUS OF THE MACHINE. DON'T START THE THREADS IF THE THREADS 
				// ARE STILL RUNNING FROM THE PREVIOUS ITERATION.

				alarm_interval += 1;
				if(alarm_interval == 3){
					status.validate();
					alarm_interval = 0;
				}

				interval = 0;
				startThreads();
			}

			// Visualize the current frames
			if(cam_side2 != 0){
				showFrames(currentframe_top, currentframe_side1, currentframe_side2);
			} else {
				showFrames(currentframe_top, currentframe_side1);
			}

			if(waitKey(frame_delay) >= 0) 
				break;
			interval += frame_delay;
		}
		Logger::v("Handler stopped!");
	}

private:
	CoffeeMakerStatus status; // Holds the status of the machine (has coffee? has filter? ...)
	CoffeeMakerPosition position; // Holds the position of the machine

	VideoCapture* cam_top; // Top camera source
	VideoCapture* cam_side1; // Side camera source
	VideoCapture* cam_side2; // Second side camera source (optional)

	Mat currentframe_top; // The current frame being executed (top)
	Mat currentframe_side1; // The current frame being executed (side 1)
	Mat currentframe_side2; // The current frame being executed (side 2)

	// Threads
	thread* coffeecan_thread;
	thread* coffeefilterholder_thread;
	thread* coffee_thread;
	thread* machinerunning_thread;
	thread* machineon_thread;
	thread* reservoiropened_thread;
	thread* coffeefilter_thread;
	thread* water_thread;
	// Number of executing threads. This is used to determine 
	int runningthreads; 

	// Mutex for synchronizing the thread
	std::mutex sideframe_mutex;
	std::mutex topframe_mutex;
	std::mutex threadend_mutex;
	std::mutex position_mutex;

	// Function to start the threads. Which threads to start depends on the status
	// of the machine. When a thread is running, the corresponding output window is shown.
	void startThreads(){		
		// Always start these four threads
		runningthreads = 4;
		coffeecan_thread = new thread(CoffeeCanThread::exec, ref(*this)); 
		coffeefilterholder_thread = new thread(CoffeeFilterHolderThread::exec, ref(*this));
		machineon_thread = new thread(MachineOnThread::exec, ref(*this));
		reservoiropened_thread = new thread(ReservoirOpenedThread::exec, ref(*this));

		// Start conditional threads
		if(status.getReservoirOpenedState()){ // If the reservoir is open, it's possible they will put water inside
			runningthreads += 1;

			if(!water_window_open){
				showWaterWindow();
			}

			water_thread = new thread(WaterThread::exec, ref(*this));
		} else {
			if(water_window_open){
				hideWaterWindow();
			}
		}

		if(status.getCoffeeFilterHolderState()){ // If the coffeefilter is outside of the machine, it's possible they'll put a filter inside		
			runningthreads += 1;
			if(status.getHasFilterState()){ // If there is a filter, we need to detect if they put coffee inside

				if(!coffee_window_open){
					showCoffeeWindow();
				}

				if(coffeefilter_window_open){
					hideCoffeeFilterWindow();
				}


				coffee_thread = new thread(CoffeeThread::exec, ref(*this));			
			} else { // We don't have a coffee filter yet, so detect a coffee filter

				if(! coffeefilter_window_open){
					showCoffeeFilterWindow();
				}

				if(coffee_window_open){
					hideCoffeeWindow();
				}

				coffeefilter_thread = new thread(CoffeeFilterThread::exec, ref(*this));
			}
		} else {
			if(coffee_window_open){
				hideCoffeeWindow();
			}

			if(coffeefilter_window_open){
				hideCoffeeFilterWindow();
			}
		}

		if(status.getMachineOnState()){ // When the machine has been turned on, detect if the machine is still running
			runningthreads += 1;

			if(!machinerunning_window_open){
				showMachineRunningWindow();
			}
			machinerunning_thread = new thread(MachineRunningThread::exec, ref(*this));
		} else {
			if(machinerunning_window_open){
				hideMachineRunningWindow();
			}
		}
	}

	// Holds the window width of the output frame 
	int framewindow_width;

	// Open, resize and move the windows that are always shown
	void showFixedWindows(){
		namedWindow(cam_window_name, CV_WINDOW_KEEPRATIO );
		namedWindow(cam_coffeecan_name, CV_WINDOW_KEEPRATIO );
		namedWindow(cam_coffeefilterholder_name, CV_WINDOW_KEEPRATIO);
		namedWindow(cam_reservoiropened_name, CV_WINDOW_KEEPRATIO);
		namedWindow(cam_machineon_name, CV_WINDOW_KEEPRATIO);

		// GET THE FIRST FRAME TO DETERMINE THE WINDOW SIZE
		Mat frame;
		*cam_top >> frame; // ALL 3 TO MAKE SURE THEY STAY IN SYNC
		*cam_side1 >> frame;
		if(cam_side2 != 0){
			*cam_side2 >> frame;
		}

		int totalwidth;
		if(cam_side2 == 0){
			totalwidth = frame.cols;
		} else {
			totalwidth = frame.cols*2;
		}
		int totalheight = frame.rows*2;

		int height = windowheight;
		double ratio = (double)height /(double)totalheight;
		framewindow_width = (int)((double)totalwidth * ratio);

		resizeWindow(cam_window_name, framewindow_width, height);
		resizeWindow(cam_coffeecan_name, (cam_side2 != 0)? 400: 200, 180);
		resizeWindow(cam_coffeefilterholder_name, 200, 180);
		resizeWindow(cam_reservoiropened_name, 200, 180);
		resizeWindow(cam_machineon_name, 200, 180);

		moveWindow(cam_window_name, 0, 0);
		moveWindow(cam_coffeefilterholder_name, framewindow_width+ 10, 0);
		moveWindow(cam_coffeecan_name, framewindow_width+10, 220);
		moveWindow(cam_reservoiropened_name, framewindow_width+ 10 + 210, 0);
		moveWindow(cam_machineon_name, framewindow_width+ 10 + 210 + 210, 0);
	}

	/*
	The following attributes and functions are used to show or hide 
	the windows with the thread output depending on the running state
	of the thread
	*/
	bool coffeefilter_window_open;
	bool water_window_open;
	bool machinerunning_window_open;
	bool coffee_window_open;

	void showCoffeeWindow(){
		coffee_window_open = true;
		namedWindow(cam_coffee_name, CV_WINDOW_KEEPRATIO);
		resizeWindow(cam_coffee_name, 200, 180);
		moveWindow(cam_coffee_name, framewindow_width+ 10, 440);
	}

	void hideCoffeeWindow(){
		coffee_window_open = false;
		destroyWindow(cam_coffee_name);
	}

	void showMachineRunningWindow(){
		machinerunning_window_open = true;
		namedWindow(cam_machinerunning_name, CV_WINDOW_KEEPRATIO);
		resizeWindow(cam_machinerunning_name, 200, 180);
		moveWindow(cam_machinerunning_name, framewindow_width+ 10 + 210, 440);
	}

	void hideMachineRunningWindow(){
		machinerunning_window_open = false;
		destroyWindow(cam_machinerunning_name);
	}

	void showCoffeeFilterWindow(){
		coffeefilter_window_open = true;
		namedWindow(cam_coffeefilter_name, CV_WINDOW_KEEPRATIO);
		resizeWindow(cam_coffeefilter_name, 200, 180);
		moveWindow(cam_coffeefilter_name, framewindow_width+ 10, 440);
	}

	void hideCoffeeFilterWindow(){
		coffeefilter_window_open = false;
		destroyWindow(cam_coffeefilter_name);
	}

	void showWaterWindow(){
		water_window_open = true;
		namedWindow(cam_water_name, CV_WINDOW_KEEPRATIO);
		resizeWindow(cam_water_name, (cam_side2 != 0)? 400: 200, 180);
		moveWindow(cam_water_name, framewindow_width+10 +  ((cam_side2 != 0)? 420: 210), 220);
	}

	void hideWaterWindow(){
		water_window_open = false;
		destroyWindow(cam_water_name);
	}

	/*
	The following functions are used to write output to one of the windows
	*/

	void showCoffeeFilterHolderAlgo(const Mat& frame_top){
		imshow(cam_coffeefilterholder_name, frame_top);
	}

	void showCoffeeFilterAlgo(const Mat& frame_top){
		imshow(cam_coffeefilter_name, frame_top);
	}

	void showCoffeeAlgo(const Mat& frame_top){
		imshow(cam_coffee_name, frame_top);
	}

	void showMachineRunningAlgo(const Mat& frame_top){
		imshow(cam_machinerunning_name, frame_top);
	}

	void showMachineOnAlgo(const Mat& frame_top){
		imshow(cam_machineon_name, frame_top);
	}

	void showReservoirOpenedAlgo(const Mat& frame_top){
		imshow(cam_reservoiropened_name, frame_top);
	}

	void showCoffeeCanAlgo(const Mat& frame_side){
		imshow(cam_coffeecan_name, frame_side);
	}

	void showCoffeeCanAlgo(const Mat& frame_left, const Mat& frame_right){
		int framecols = frame_left.cols;
		int framerows = frame_left.rows;
		int frametype = frame_left.type();

		Mat imgResult = Mat::zeros(framerows,2*framecols,frametype);

		Mat roiImgResult_Left = imgResult(Rect(0,0,framecols,framerows));
		Mat roiImgResult_Right = imgResult(Rect(framecols,0,framecols,framerows));

		frame_right.copyTo(roiImgResult_Left); 
		frame_left.copyTo(roiImgResult_Right); 

		imshow(cam_coffeecan_name, imgResult);
	}

	void showWaterAlgo(const Mat& frame_side){
		imshow(cam_water_name, frame_side);
	}

	void showWaterAlgo(const Mat& frame_left, const Mat& frame_right){
		int framecols = frame_left.cols;
		int framerows = frame_left.rows;
		int frametype = frame_left.type();

		Mat imgResult = Mat::zeros(framerows,2*framecols,frametype);

		Mat roiImgResult_Left = imgResult(Rect(0,0,framecols,framerows));
		Mat roiImgResult_Right = imgResult(Rect(framecols,0,framecols,framerows));

		frame_right.copyTo(roiImgResult_Left); 
		frame_left.copyTo(roiImgResult_Right); 

		imshow(cam_water_name, imgResult);
	}

	void showFrames(const Mat& frame_top, const Mat& frame_side1, const Mat& frame_side2){
		int framecols = frame_top.cols;
		int framerows = frame_top.rows;
		int frametype = frame_top.type();

		Mat imgResult = Mat::zeros(2*framerows,2*framecols,frametype);

		Mat roiImgResult_Side1 = imgResult(Rect(0,framerows,framecols,framerows));
		Mat roiImgResult_Side2 = imgResult(Rect(framecols,framerows,framecols,framerows));
		Mat roiImgResult_Top = imgResult(Rect(0,0,framecols,framerows));

		frame_side2.copyTo(roiImgResult_Side1); 
		frame_side1.copyTo(roiImgResult_Side2); 
		frame_top.copyTo(roiImgResult_Top);

		imshow(cam_window_name, imgResult);
	}

	void showFrames(const Mat& frame_top, const Mat& frame_side){
		int framecols = frame_top.cols;
		int framerows = frame_top.rows;
		int frametype = frame_top.type();

		Mat imgResult = Mat::zeros(2*framerows,framecols,frametype);

		Mat roiImgResult_Side = imgResult(Rect(0,framerows,framecols,framerows));
		Mat roiImgResult_Top = imgResult(Rect(0,0,framecols,framerows));

		frame_side.copyTo(roiImgResult_Side); 
		frame_top.copyTo(roiImgResult_Top);

		imshow(cam_window_name, imgResult);
	}

};

#endif
