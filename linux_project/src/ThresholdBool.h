#ifndef ThresholdBool_H
#define ThresholdBool_H

// The threshold bool is a modification to the bool principle. Every time the
// threshold bool's value is set with true or false, the inner integer value
// of the bool is increased or decreased with the value of 1. When the inner
// value exceeds the value of threshold/2, then the threshold bool is considered
// true, otherwise false.
//
// Using this kind of booleans to store the machine status booleans, false possitives
// are filtered out, because one value doesn't necessarily change the value of the
// threshold bool.
class ThresholdBool
{
public: 
	ThresholdBool(int threshold, bool reversable) : value(0), threshold(threshold), reversable(reversable) {
	}

	ThresholdBool(int threshold, bool on, bool reversable) : threshold(threshold), reversable(reversable){
		if(on){
			value = threshold;
		} else {
			value = 0;
		}
	}

	operator const bool() const {
            return (double)value > (threshold /2.0);
        }

	ThresholdBool & operator = (bool v){
		if(v){
			value += 1;
		} else if(!v && reversable) {
			value -= 1;
		}

		if(value > threshold){
			value = threshold;
		}

		if(value < 0){
			value = 0;
		}

		return *this;
	}

	ThresholdBool & operator &= (bool v){
		if(v){
			value = threshold;
		} else {
			value = 0;
		}

		return *this;
	}

private:
	int threshold;
	int value;
	bool reversable;
};

#endif