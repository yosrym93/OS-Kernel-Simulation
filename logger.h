#include <fstream>  
using namespace std;


class Logger
{
private:
    ofstream myfile;
    int* logTime; 
public:
    void initialize(string file, int& currentTime);
    void log(string data);
    void terminate();  
};

void Logger::initialize(string file, int& currentTime)
{
    myfile.open(file);
    logTime = &currentTime;
}


void Logger::log(string data)
{
    myfile << *logTime << "\t" << data << endl;
}

void Logger::terminate()
{
    myfile.close();
}
