#include <fstream>  
using namespace std;


class Logger
{
private:
    ofstream myfile;
    int* logTime; 
public:
    void initializeLogger(string file, int& currentTime);
    void log(string data);
    void terminateLogger();  
};

void Logger::initializeLogger(string file, int& currentTime)
{
    myfile.open(file);
    logTime = &currentTime;
}


void Logger::log(string data)
{
    myfile << *logTime << "\t" << data << endl;
}

void Logger::terminateLogger()
{
    myfile.close();
}
