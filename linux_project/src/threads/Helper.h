#ifndef HELPER_H
#define HELPER_H

// This class contains the shared helper functions used in several threads
class Helper{
public:
	// Convert the frame to HSV
	static Mat convertToHSV(const Mat& inputFrame){
		Mat hsvImage;
		cvtColor(inputFrame, hsvImage, CV_RGB2HSV);
		return hsvImage;
	}

	// Filter out colors in a specific range
	static Mat filterColor(const Mat& frame, const Scalar& lowerBound, const Scalar& upperBound){
		Mat result;
		inRange(frame, lowerBound, upperBound, result);
		return result;
	}

	// Crop the frame to a specific area
	static Mat crop(const Mat & img, const Rect& area){
		return Mat(img,area);
	}

	// Detect the coffee filter holder in the specific frame.
	static Vec3f findCoffeeHolder(const Mat & img, Mat& result, CoffeeMakerPosition & pos){
		Mat blur;

		// Crop the image 
		Mat cropped;
		cropped = Helper::crop(img, Rect(Point(0,0),Point(img.cols,pos.getY()-pos.getRatio()*100)));
	
		// Filter image to leave only red
		Mat filtered = convertToHSV(img); 
		filtered = Helper::filterColor(filtered, Scalar(0,208,166),Scalar(98,256,256));
		medianBlur(filtered,filtered,5);
		
		// Closing operation to get a nicer circle
		Mat dilater = getStructuringElement(MORPH_ELLIPSE,Size(10,10));
		result = Mat::zeros(img.rows,img.cols,CV_8UC3);

		Vec3f holder;
		vector<Vec3f> circles;
		int min_diameter = 10;
		int max_diameter = 200;
		int offset = 200;

		HoughCircles(filtered,circles,CV_HOUGH_GRADIENT,1,500,20,1,min_diameter,max_diameter);

		// When a circle is found and the diameter comes close to the expected size of the holder,
		// then the filter holder is found and its position is returned.
		if(circles.size()==1 && cvRound(circles[0][1]) < offset){	
			Point c(cvRound(circles[0][0]), cvRound(circles[0][1]));
			int r = cvRound(circles[0][2]);
			circle( result, c, r, Scalar(0,0,255), 4, 8, 0 );
			holder = circles[0];
		}

		return holder;
	}

	// When the coffeefilter holder is found, this function cas be used to detect it's
	// content:
	// 1: Only the filter holder
	// 2: Filter holder + filter
	// 3: Filter holder + filter + coffee
	static int getTypeFilter(const Mat & gray, Mat & result, const Vec3f & middle, const int fault){
		int count = 0;
		int intensity = 0;

		int min_x,max_x,min_y,max_y;
		min_x = middle[0]-fault;
		max_x = middle[0]+fault;
		min_y = middle[1]-fault;
		max_y = middle[1]+fault;

		if(min_x < 0)
			min_x = 0;
		if(max_x > gray.size().width)
			max_x = gray.size().width;
		if(min_y < 0)
			min_y = 0;
		if(max_y > gray.size().height)
			max_y = gray.size().height;

		for(int i = min_x; i < max_x;i++){
			for(int j = min_y; j < max_y; j++){
				Scalar bg = gray.at<uchar>(j,i);
				intensity += bg[0];
				count++;
			} 
		}

		int average = intensity/count;
		Point c(cvRound(middle[0]), cvRound(middle[1]));
		int r = 1;
		circle( result, c, r, Scalar(average,average,average), cvRound(middle[2]*1.8), 8, 0 );

		if(average < 80)
			return 1;
		else if(average > 140)
			return 2;
		else
			return 3;
	}

	// Helper function to detect coffee or filter inside coffeefilter holder
	static Mat validateCoffeeOrFilter(bool & gedetecteerd, const Mat & img, bool koffie, CoffeeMakerPosition & pos){
		// Find holder
		Mat result, gray;
		cvtColor(img,gray,CV_BGR2GRAY);
		Vec3f holder = Helper::findCoffeeHolder(img,result, pos); 
		
		// If holder found
		if(holder[2] > 0)
			if(koffie){
				if(getTypeFilter(gray,result,holder,40) == 3)
					gedetecteerd = true; 	
			}
			else{
				if(getTypeFilter(gray,result,holder,40) == 2)
					gedetecteerd = true; 	
			}
		return result;
	}
};

#endif