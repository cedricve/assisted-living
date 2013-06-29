#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <ctime>

using namespace std;

// The Logger functions are used to seperate the output in seperate streams
// When the logger is set to VERBOSE then all the output will be printed.
// Otherwise, only alarm, error and state functions will be printed.
// Set verbose to true in the main class, to retrieve all output!

namespace Logger
{
	static bool verbose = true;

	static string timestamp(){
		time_t now = time(NULL);
		struct tm * ptm = localtime(&now);
		char buffer[32];

		strftime (buffer, 16, "%H:%M:%S", ptm);

		return string("[") + buffer +string("]");
	}

	static void setVerbose(bool v){
		Logger::verbose = v;
	}

	// Function used for alarm output
	static void a(string text){
		cout << Logger::timestamp() << "[ALARM]: " << text << endl;
	}

	// Function used for verbose output
	static void v(string text){
		if(Logger::verbose){
			cout << Logger::timestamp() << "[INFO]: " << text << endl;
		}
	}

	// Function used for non verbose output
	static void i(string text, bool addtimestamp = true){
		if(addtimestamp){
			cout << Logger::timestamp() << " ";
		}
		cout <<  text << endl;
	}

	// Function used when an error occured
	static void e(string text){
		cout << Logger::timestamp() << "[ERROR]: " << text << endl;
	}

	// Function used when the state of the system changes
	static void s(string text){
		cout << Logger::timestamp() << "[STATE]: " << text << endl;
	}
};

#endif
