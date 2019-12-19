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

key_t fromDiskQueueID, toDiskQueueID;

struct msgbuff {
    long mtype;
    char data[MSG_SIZE];
};

void SIGUSR1_handler(int signum) {
    msgbuff response;
    response.mtype = 1;
    strncpy(response.data, "1", 1);
    msgsnd(fromDiskQueueID, &response, MSG_SIZE, IPC_NOWAIT);
    cout << "1 free space available" << endl;
    msgrcv(toDiskQueueID, &response, MSG_SIZE, 0, 0); // Blocking call, should not enter if no space is available
    cout << "Message received, mtype = " << response.mtype << " data = " << response.data << endl;
    kill(getppid(), SIGUSR1);
}

void SIGUSR2_handler(int signum) {
}

int main(int argc, char* argv[]) {
    toDiskQueueID = key_t(stoi(argv[1]));
    fromDiskQueueID =  key_t(stoi(argv[2]));
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGUSR2, SIGUSR2_handler);
    while(true) {};
}