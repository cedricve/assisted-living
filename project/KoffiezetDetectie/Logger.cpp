#include "Logger.h"

bool Logger::verbose = false;

// The Logger class is used to seperate the output in seperate streams
// When the logger is set to VERBOSE then all the output will be printed.
// Otherwise, only alarm, error and state functions will be printed.
// Set verbose to true in the main class, to retrieve all output!

void Logger::setVerbose(bool v){
	Logger::verbose = v;
}

// Function used for verbose output
void Logger::a(string text){
	cout << timestamp() << "[ALARM]: " << text << endl;
}

// Function used for verbose output
void Logger::v(string text){
	if(Logger::verbose){

		cout << timestamp() << "[INFO]: " << text << endl;
	}
}

// Function used for non verbose output

void Logger::i(string text, bool addtimestamp){
	if(addtimestamp){
		cout << timestamp() << " ";
	}
	 cout <<  text << endl;
}

// Function used when an error occured
void Logger::e(string text){
	cout <<timestamp() << "[ERROR]: " << text << endl;
}

// Function used when the state of the system changes
void Logger::s(string text){
	cout <<timestamp() << "[STATE]: " << text << endl;
}

// Calcultate the current time stamp
string Logger::timestamp(){
	time_t now = time(NULL);
	struct tm * ptm = localtime(&now);
	char buffer[32];

	strftime (buffer, 16, "%H:%M:%S", ptm);

	return string("[") + buffer +string("]");
}