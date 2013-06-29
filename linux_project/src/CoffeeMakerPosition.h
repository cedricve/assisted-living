#ifndef COFFEEMAKER_POSITION_H
#define COFFEEMAKER_POSITION_H

// This class contains the positioning information of the coffeemachine.
// The values are calculated during the initialization of the handler class. 
// When calibration is done, using these values you can determine the exact
// information of the machine such as the zoom factor (how big is the machine in
// the image, or the rotation.
//
// This information is used to accurately select regions to crop when using certain
// algorithms in the threads.
class CoffeeMakerPosition{
public:
	CoffeeMakerPosition()
		: x(0), y(0), width(0), height(0), ratio(0), angle(0) {
	}

	CoffeeMakerPosition(int x, int y, int width, int height, double ratio, double angle)
		: x(x), y(y), width(width), height(height), ratio(ratio), angle(angle){
	}

	int getX(){
		return x;
	}

	int getY(){
		return y;
	}

	int getWidth(){
		return width;
	}

	int getHeight(){
		return height;
	}

	double getRatio(){
		return ratio;
	}

	double getAngle(){
		return angle;
	}

	friend ostream& operator<<(ostream& os, const CoffeeMakerPosition& pt);

private:
	int x, y;
	int width, height;
	double ratio;
	double angle;
};

ostream& operator<<(ostream& os, const CoffeeMakerPosition& dt)
{
    os << "Center:\n\tX: " << dt.x << "\n\tY: " << dt.y << endl;
	os << "Size:\n\tWidth: " << dt.width << "\n\tHeight: " << dt.height << endl;
	os << "Angle: " << dt.angle << endl;
	os << "Ratio: " << dt.ratio << endl;

    return os;
}

#endif