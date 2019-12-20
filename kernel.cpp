#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <wait.h>
#include <queue>
#include "logger.h"

#define PROCESSES_COUNT 2
#define PROCESS_EXECUTABLE_FILE "process"
#define DISK_EXECUTABLE_FILE "disk"
#define PROCESS_INPUT_FILE_NAME "processInput"
#define LOG_FILE_NAME "log.txt"
#define ADD_REQ 1
#define DELETE_REQ 2
#define MSG_SIZE 30
#define MSG_QUEUE_FLAGS 0666

using namespace std;

struct msgbuff {
    long mtype;
    char data[MSG_SIZE];
};

const int DATA_SIZE = sizeof(msgbuff) - sizeof(long);
int currentTime, runningProcesses;
bool isDiskAvailable, processesRunning;
Logger logger;

void setUpSignalHandlers();
pid_t createDisk(string diskFilePath, key_t toDiskQueueID, key_t fromDiskQueueID);
void createProcesses(string processFilePath, string processInputFile, 
                     int processesCount, key_t requestsQueueID);
void getRequests(queue<msgbuff>& requests, key_t requestsQueueID);
void sendDiskOperation(queue<msgbuff>& requests, key_t toDiskQueueID, 
                       key_t fromDiskQueueID, pid_t diskPID);
void createMessageQueues(key_t& requestsQueueID, key_t& toDiskQueueID, key_t& fromDiskQueueID);
void terminateMessageQueues(key_t& requestsQueueID, key_t& toDiskQueueID, key_t& fromDiskQueueID);
void terminateDisk(pid_t diskPID);
void logTerminating();


int main() {
    logger.openFile(LOG_FILE_NAME);
    currentTime = 1;
    isDiskAvailable = true;
    processesRunning = true;
    key_t requestsQueueID, toDiskQueueID, fromDiskQueueID;
    queue<msgbuff> requests;
    createMessageQueues(requestsQueueID, toDiskQueueID, fromDiskQueueID);
    setUpSignalHandlers();
    int diskPID = createDisk(DISK_EXECUTABLE_FILE, toDiskQueueID, fromDiskQueueID);
    createProcesses(PROCESS_EXECUTABLE_FILE, PROCESS_INPUT_FILE_NAME, PROCESSES_COUNT, requestsQueueID);
    alarm(1);
    while(processesRunning || !requests.empty() || !isDiskAvailable) {
        getRequests(requests, requestsQueueID);
        sendDiskOperation(requests, toDiskQueueID, fromDiskQueueID, diskPID);
    }
    terminateDisk(diskPID);
    terminateMessageQueues(requestsQueueID, toDiskQueueID, fromDiskQueueID);
    logTerminating();
    logger.closeFile();
}

/************** Termination Handling **************/

void terminateDisk(pid_t diskPID) {
    kill(diskPID, SIGKILL);
}

void terminateMessageQueues(key_t& requestsQueueID, key_t& toDiskQueueID, key_t& fromDiskQueueID) {
    msgctl(requestsQueueID, IPC_RMID, nullptr);
    msgctl(toDiskQueueID, IPC_RMID, nullptr);
    msgctl(fromDiskQueueID, IPC_RMID, nullptr);
}

/************** Logging **************/

void logRequestReceived(const msgbuff& request) {
    string logData = "Request received, type: ";
    logData += request.mtype == ADD_REQ ? "ADD" : "DEL";
    logData += ", data: ";
    logData += request.data;
    logger.log(logData);
}

void logFreeSlotsChecked() {
    string logData = "Disk is being checked for free slots";
    logger.log(logData);
}

void logCheckDiskResponse(const msgbuff& response) {
    string logData = "Disk has ";
    logData += response.data[0];
    logData += " free slot(s)";
    logger.log(logData);
}

void logRequestSentToDisk(const msgbuff& request) {
    string logData = "Request sent to disk, type: ";
    logData += request.mtype == ADD_REQ ? "ADD" : "DEL";
    logData += ", data: ";
    logData += request.data;
    logger.log(logData);
}

void logTerminating() {
    logger.log("All processes exited and all requests are served, exiting..");
}

/************** Requests Handling **************/

bool checkRequest(key_t requestsQueueID, msgbuff& request) {
    int status = msgrcv(requestsQueueID, &request, DATA_SIZE, 0, IPC_NOWAIT);
    if(status != -1) {
        return true;
    }
    else {
        return false;
    }
} 

void getRequests(queue<msgbuff>& requests, key_t requestsQueueID) {
    msgbuff request;
    while(checkRequest(requestsQueueID, request)) {
        logRequestReceived(request);
        requests.push(request);
    }
}

void clearAddRequests(queue<msgbuff>& requests) {
    while(!requests.empty() && requests.front().mtype == ADD_REQ) {
        requests.pop();
    }
}

/************** Disk Operations **************/

bool checkDiskFreeSlots(key_t fromDiskQueueID, pid_t diskPID) {
    kill(diskPID, SIGUSR1);
    logFreeSlotsChecked();
    msgbuff diskResponse;
    int status = msgrcv(fromDiskQueueID, &diskResponse, DATA_SIZE, 0, 0);
    int freeSlots = int(diskResponse.data[0]) - int('0');
    logCheckDiskResponse(diskResponse);
    return freeSlots > 0;
}

void sendToDisk(key_t toDiskQueueID, msgbuff* msg) {
    logRequestSentToDisk(*msg);
    msgsnd(toDiskQueueID, msg, DATA_SIZE, IPC_NOWAIT);
}

void sendDiskOperation(queue<msgbuff>& requests, 
                       key_t toDiskQueueID, 
                       key_t fromDiskQueueID,
                       pid_t diskPID) {
    if(isDiskAvailable && !requests.empty()) {
        auto currentRequest = requests.front();
        requests.pop();
        if(currentRequest.mtype == ADD_REQ && !checkDiskFreeSlots(fromDiskQueueID, diskPID)) {
            clearAddRequests(requests);
            return;
        }
        sendToDisk(toDiskQueueID, &currentRequest);
        isDiskAvailable = false;
    }
}

/************** Message Queues Creation **************/

void createMessageQueues(key_t& requestsQueueID, 
                         key_t& toDiskQueueID, 
                         key_t& fromDiskQueueID)
{
    requestsQueueID = msgget(IPC_PRIVATE, IPC_CREAT  | MSG_QUEUE_FLAGS);
    toDiskQueueID = msgget(IPC_PRIVATE, IPC_CREAT  | MSG_QUEUE_FLAGS);
    fromDiskQueueID = msgget(IPC_PRIVATE, IPC_CREAT  | MSG_QUEUE_FLAGS);
}

/************** Signals Handling **************/

void SIGUSR1_handler(int signum) {
    isDiskAvailable = true;
}

void SIGUSR2_handler(int signum) {
    currentTime++;
}

void SIGCHLD_handler(int signum) {
    int status;
    while(waitpid(-1, &status, WNOHANG) > 0) {
        runningProcesses--;
    }
}

void SIGALRM_handler(int signum) {
    if(runningProcesses == 0)
        processesRunning = false;
    else {
        alarm(1);
        killpg(getpgrp(), SIGUSR2);
    }
}

void setUpSignalHandlers() {
    signal(SIGUSR2, SIGUSR2_handler);
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGCHLD, SIGCHLD_handler);
    signal(SIGALRM, SIGALRM_handler);
}

/************** Process Creation **************/

pid_t createDisk(string diskFilePath, key_t toDiskQueueID, key_t fromDiskQueueID) {
    pid_t diskPID = fork();
    if(diskPID == 0) {
        execl(diskFilePath.c_str(), diskFilePath.c_str(), 
              to_string(toDiskQueueID).c_str(), to_string(fromDiskQueueID).c_str() , nullptr);
    }
    else {
        return diskPID;
    }
}

void createProcess(string processFilePath, string processInputFile, key_t requestsQueueID) {
    pid_t pid = fork();
    if(pid == 0) {
        execl(processFilePath.c_str(), processFilePath.c_str(), 
              processInputFile.c_str(), to_string(requestsQueueID).c_str(), nullptr);
    }
}

void createProcesses(string processFilePath, string inputFilesName, 
                     int processesCount, key_t requestsQueueID)
{
    runningProcesses = processesCount;
    while(processesCount--) {
        string inputFileName = inputFilesName + to_string(processesCount) + ".txt";
        createProcess(processFilePath, inputFileName, requestsQueueID);
    }
}