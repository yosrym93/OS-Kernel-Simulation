#include <iostream>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>

#define MSG_SIZE 30
#define FROM_DISK_QUEUE_KEY 1997
#define TO_DISK_QUEUE_KEY 1998

using namespace std;

struct msgbuff {
    long mtype;
    char data[MSG_SIZE];
};

const int DATA_SIZE = sizeof(msgbuff) - sizeof(long);
key_t fromDiskQueueID, toDiskQueueID;

void SIGUSR1_handler(int signum) {
    msgbuff response;
    response.mtype = 1;
    strncpy(response.data, "1", 1);
    msgsnd(fromDiskQueueID, &response, DATA_SIZE, IPC_NOWAIT);
    msgrcv(toDiskQueueID, &response, DATA_SIZE, 0, 0); // Blocking call, should not enter if no space is available
    kill(getppid(), SIGUSR1);
}

void SIGUSR2_handler(int signum) {
}

int main(int argc, char* argv[]) {
    toDiskQueueID = key_t(stoi(argv[1]));
    fromDiskQueueID =  key_t(stoi(argv[2]));
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGUSR2, SIGUSR2_handler);
    msgbuff response;
    while(true) {
        int status = msgrcv(toDiskQueueID, &response, DATA_SIZE, 0, IPC_NOWAIT);
        if(status != -1)
            kill(getppid(), SIGUSR1);
    };
}