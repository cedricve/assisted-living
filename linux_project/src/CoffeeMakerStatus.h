#ifndef COFFEEMAKER_STATUS_H
#define COFFEEMAKER_STATUS_H

#include "ThresholdBool.h"
#include "Logger.h"

// The following contants contain the default value for all the ThresholdBools
const int HASCOFFEECAN_THRESH = 6;
const int RESERVOIROPEN_THRESH = 10;
const int MACHINERUNNING_THRESH = 18;
const int MACHINEON_THRESH = 10;
const int COFFEEFILTERHOLDER_THRESH = 5;
const int COFFEE_THRESH = 5;
const int HASFILTER_THRESH = 3;
const int HASWATER_THRESH = 10;

// This class contains the current status of the coffee machine.
// After a thread return it's result to the handler class, the handler
// shall change the current status of the machine. To handle wrong
// values ThresholdBools are used to make sure changes in the state
// of the machine don't happen immediately, but only when the machine
// is sure about the result.
class CoffeeMakerStatus
{
public:
	CoffeeMakerStatus() : hascoffeecan(HASCOFFEECAN_THRESH, true, true), reservoiropen(RESERVOIROPEN_THRESH, true), machinerunning(MACHINERUNNING_THRESH, true), machineon(MACHINEON_THRESH, true), coffeefilterholder(COFFEEFILTERHOLDER_THRESH, true), hascoffee(COFFEE_THRESH, false), hasfilter(HASFILTER_THRESH, false), haswater(HASWATER_THRESH, false) {
	}

	void setHasCoffeeCanState(bool result){
		bool temp = hascoffeecan;

		hascoffeecan = result;

		if(temp != hascoffeecan){
			if(hascoffeecan){
				Logger::s("+ CoffeeCan has been put INSIDE of the machine.");
			} else {
				Logger::s("- CoffeeCan has been taken OUTSIDE of the machine.");
			}
		}
	}

	void setReservoirOpenedState(bool result){
		bool temp = reservoiropen;

		reservoiropen = result;

		if(temp != reservoiropen){
			if(reservoiropen){
				Logger::s("+ Water reservoir has been OPENED.");
			} else {
				Logger::s("- Water reservoir has been CLOSED.");
			}
		}
	}

	bool getReservoirOpenedState(){
		return reservoiropen;
	}

	void setMachineRunningState(bool result){
		bool temp = machinerunning;

		machinerunning = result;

		if(temp != machinerunning){
			if(machinerunning){
				Logger::s("+ Machine STARTED working.");
			} else {
				Logger::s("- Machine STOPPED working.");
			}
		}
	}

	void setMachineOnState(bool result){
		bool temp = machineon;

		machineon = result;

		if(temp != machineon){
			if(machineon){
				Logger::s("+ Machine has been turned ON.");
			} else {
				Logger::s("- Machine has been turned OFF.");
			}
		}
	}

	bool getMachineOnState(){
		return machineon;
	}

	void setCoffeeFilterHolderState(bool result){
		bool temp = coffeefilterholder;

		coffeefilterholder = result;

		if(temp != coffeefilterholder){
			if(coffeefilterholder){
				Logger::s("+ CoffeeFilter holder has been taken OUTSIDE of the machine.");
			} else {
				Logger::s("- CoffeeFilter holder has been put INSIDE of the machine.");
			}
		}
	}

	bool getCoffeeFilterHolderState(){
		return coffeefilterholder;
	}

	void setHasCoffeeState(bool result){
		bool temp = hascoffee;

		hascoffee = result;

		if(temp != hascoffee){
			if(hascoffee){
				Logger::s("+ Coffee has been put INSIDE of the filter.");
			} 
		}
	}

	void setHasFilterState(bool result){
		bool temp = hasfilter;

		hasfilter = result;

		if(temp != hasfilter){
			if(hasfilter){
				Logger::s("+ A filter has been put INSIDE the CoffeeFilter holder.");
			}
		}
	}

	bool getHasFilterState(){
		return hasfilter;
	}

	void setWaterState(bool result){
		bool temp = haswater;

		haswater = result;

		if(temp != haswater){
			if(haswater){
				Logger::s("+ The machine has been filled with water.");
			}
		}
	}

	// This function returns true or false indicating if the current state of the current machine is valid
	// When the machine is running and the current state is not valid, an alarm message will be shown indicating
	// the specific error
	bool validate(){ 
		if(machineon && machinerunning){
			bool valid = true;
			if(!hascoffeecan){
				valid = false;
				Logger::a("!! The coffeecan needs to be inside the machine!");
			}

			if(!hasfilter){
				valid = false;
				Logger::a("!! There is no coffee filter inside the coffee filter holder!");
			}

			if(!hascoffee) {
				valid = false;
				Logger::a("!! There is no coffee inside the machine!");
			}

			if(reservoiropen){
				valid = false;
				Logger::a("!! The water reservoir is still opened!");
			}

			if(coffeefilterholder){
				valid = false;
				Logger::a("!! The coffee filter holder is outside of the machine!");
			}

			if(!haswater || (!machinerunning)){ 
				valid = false;
				Logger::a("!! There is not enough water in the machine!");
			}

			return valid;
			
		} else {
			return true;
		}
	}

private:
	ThresholdBool hascoffeecan;
	ThresholdBool reservoiropen;
	ThresholdBool machinerunning;
	ThresholdBool machineon;
	ThresholdBool coffeefilterholder;
	ThresholdBool hascoffee;
	ThresholdBool hasfilter;
	ThresholdBool haswater;
};

#endif