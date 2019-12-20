#include<iostream>
#include <fstream>  
#include"string"
using namespace std;


class logger
{
private:
    ofstream myfile; 
public:
    logger();
    void openFile(string f);
    void writeToFile(string data);
    void closeFile();  
    ~logger();
};

logger::logger()
{}

logger::~logger()
{}

void logger::openFile(string f)
{
    myfile.open (f);
    myfile << "New log file\n\n";
}


void logger::writeToFile(string data)
{
    myfile << data + "\n\n";
}

void logger::closeFile()
{
    myfile << "End of log file\n\n";
    
    myfile.close();
}

/*
int main()
{
    char a[4];
    a[0] = 'a';
    a[1] = 'a';
    a[2] = 'a';
    string b = "bbb";
    cout << a + b;
}
*/