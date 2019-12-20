#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <signal.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <unistd.h>

using namespace std;

// Constants used during the whole run time
#define ADD_TYPE 1
#define DELETE_TYPE 2
#define MESSAGE_TEXT_SIZE 30

// Define Time as global variable
int t;
// Define dataa struct that holds an information.
struct dataa{
int time;
string operation;
string operation_data;
};

// Define message buffer with a type and message content
struct msgbuff
{
   long mtype;
   char mtext[MESSAGE_TEXT_SIZE];
};

// handler to a user2 signal
void handler(int signum);
// Function that is used to read the file of the process and fill the vector of operations
vector<dataa> read_file(string);

//Utility 
//this function returns the type of the operation as an integer
int get_type(string );
//Utility 
// the criteria that the vector of operations are sorted with
bool sorting_criteria(dataa , dataa);
//Utility 
// sending a message through the message queue to the kernel
void send_operation(key_t,int,string);

int main(int argc,char *argv[])
{
    signal (SIGUSR2, handler);
    string filename = argv[1];
    vector<dataa> process_operations;
    process_operations = read_file(filename);
    sort(process_operations.begin(),process_operations.end(),sorting_criteria);
    // Initialize global time with one
    t = 1;
    // Getting the key of the queue as a parameter
    key_t msgid = key_t(stoi(argv[2]));
    int size;
    dataa active_operation;
    size = process_operations.size(); // Initialize the size
    // Looping until the process finishes all the process
    while(!process_operations.empty())
    {
        // Checking first process after sorting them
        if (process_operations[size - 1].time == t)
        {
            // get this process and send the message to the kernel
            active_operation = process_operations[size - 1]; // get active operation
            send_operation(msgid,get_type(active_operation.operation),active_operation.operation_data); // send operation to kernel
            process_operations.pop_back(); // remove operation
            size = process_operations.size(); // setting size with the new size
        }
    }

    return 0;
}
// Functions ==============================================================
// ========================================================================

// Function that is used to read the file of the process and fill the vector of operations
vector<dataa> read_file(string filename)
{
    ifstream inFile;
    // Open file
    inFile.open(filename);
    vector <dataa> temp;
    dataa temp_element;
    int temp_time;
    string temp_operation, temp_operation_data, temp_line;
    vector<string> result;
    while(!inFile.eof())
    {
        // Initialize operation data with empty string
        temp_operation_data = "";
        // Get the whole line in temp_line variable 
        getline(inFile, temp_line);
        // If the line is empty string end
        if(temp_line == "")
            break;
        // Seperate the line with white spaces
        // and put the strings in the result vector 
        std::istringstream iss(temp_line);
        for(std::string temp_line; iss >> temp_line; )
            result.push_back(temp_line);
        // time is the first element convert it into integer
        temp_time = atoi(result[0].c_str());

        // operation is the second element
        temp_operation = result[1];

        // put the remaining data in toperation data
        for(int i=2;i<result.size();i++)
        {
            temp_operation_data+=result[i];
            temp_operation_data+=" ";
        }
        // pysh the operation in the operation vector
        temp_element.time = temp_time;
        temp_element.operation = temp_operation;
        temp_element.operation_data = temp_operation_data;
        temp.push_back(temp_element);
        result.clear();
    }
    // close file
    inFile.close();
    // return temp vector
    return temp;
}

// signal that increment the global time (Kernel responsibility)
void handler(int signum)
{
    t++;

}
// the criteria that the vector of operations are sorted with
bool sorting_criteria(dataa first, dataa second)
{
    return (first.time > second.time);
}
// sending a message through the message queue to the kernel
void send_operation(key_t msgqid,int operation_type,string operation)
{
  struct msgbuff message;
  int send_val;
  // Set the type (ADD OR DEL)
  message.mtype = operation_type;     	
  // Put the operation data in the text field of the message 
  strncpy(message.mtext ,operation.c_str(),MESSAGE_TEXT_SIZE);
  send_val = msgsnd(msgqid, &message, sizeof(message.mtext), !IPC_NOWAIT);
  // Check if the sending was correct.
  if(send_val == -1)
  	perror("Errror in send");
}

// this function returns the type of the operation as an integer
int get_type(string type)
{
    if(type == "ADD")
        return ADD_TYPE;
    else
        return DELETE_TYPE;
}
