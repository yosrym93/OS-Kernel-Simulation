#include <fstream>  
using namespace std;


class Logger
{
private:
    ofstream myfile; 
public:
    Logger();
    void openFile(string f);
    void log(string data);
    void closeFile();  
    ~Logger();
};

Logger::Logger()
{}

Logger::~Logger()
{}

void Logger::openFile(string f)
{
    myfile.open(f);
}


void Logger::log(string data)
{
    myfile << data << endl;
}

void Logger::closeFile()
{
    myfile.close();
}
