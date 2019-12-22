#include <iostream>
#include <signal.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <cstring>
#include <unistd.h>
#include <vector>
#define MAX_DISK_SIZE 10
#define MESSAGE_TEXT_SIZE 30
#define ADD_REQ 1
#define DELETE_REQ 2
#define ADD_COST 3
#define DELETE_COST 1

using namespace std;

/************** Global and Disk's virtual Clocks **************/
int CLK = 1;
int localCLK=-1;

/************** Disk Class	**************/
class HDD
{
	vector<char*> data;
	bool isSlotFree[MAX_DISK_SIZE];
	int dataCount;
	int nextFree;
	void updateNextFree()
	{
		if (dataCount < MAX_DISK_SIZE)
			for (int i = 0; i < MAX_DISK_SIZE; i++)
			{
				if (isSlotFree[i])
				{
					nextFree = i;
					return;
				}
			}
		nextFree = -1;
		return;
	}


public:
	HDD()
	{
		data.reserve(MAX_DISK_SIZE);

		for (int i = 0; i < MAX_DISK_SIZE;i++)
		{
			char holder[MESSAGE_TEXT_SIZE];
			strncpy(holder,"null",MESSAGE_TEXT_SIZE);
			isSlotFree[i] = true;
			data.push_back(holder);
		}
		dataCount = 0;
		nextFree = 0;
	}
	void add(char* input)
	{
		if (nextFree == -1)
			return;
		data[nextFree] = input;
		strncpy(data[nextFree],input,MESSAGE_TEXT_SIZE);
		isSlotFree[nextFree] = false;
			dataCount++;
		updateNextFree();
	}
	void remove(int index)
	{
		if (index >= 0 && index < MAX_DISK_SIZE)
		{
			if (!isSlotFree[index])
			{
				isSlotFree[index] = true;
				dataCount--;
				updateNextFree();
			}
			return;
		}
	}
	int freeCount()
	{
		return MAX_DISK_SIZE - dataCount;
	}

};

/************** Disk Creation **************/
HDD disk;
/************** Message Queues Creation **************/
struct msgbuff
{
    long mtype;
    char data[MESSAGE_TEXT_SIZE];
};

const int DATA_SIZE = sizeof(msgbuff) - sizeof(long);
key_t fromDiskQueueID, toDiskQueueID;


/************** Signals Handlers **************/

void SIGUSR1_handler(int signum)
{
    if(localCLK>CLK)
        cout<<"ERROR: SIGUSR1 RECIEVED IN DISK WHILE BUSY";
    msgbuff response;
    response.data[0] = char(disk.freeCount());
    response.mtype = 1;
    msgsnd(fromDiskQueueID, &response, DATA_SIZE, IPC_NOWAIT);							//Sends the kernel how many free slots it has
}

void SIGUSR2_handler(int signum)
{
    CLK++;
    if(CLK == localCLK)																	//Notifies the kernel when done from current operation
        kill(getppid(), SIGUSR1);

}
/************** Main **************/
int main(int argc, char* argv[]) 
{
    toDiskQueueID = key_t(stoi(argv[1]));												//Defining messages keys
    fromDiskQueueID =  key_t(stoi(argv[2]));
    signal(SIGUSR1, SIGUSR1_handler);
    signal(SIGUSR2, SIGUSR2_handler);
    msgbuff response;
    while(true)
    {

        while(localCLK <= CLK)															//Loops only when not busy
        {
          
            int status = msgrcv(toDiskQueueID, &response, DATA_SIZE, 0, IPC_NOWAIT);	//Checks for any messages and acts based upon message type
            if(status != -1)
            { 
                if(response.mtype == ADD_REQ)
                {
                    localCLK = CLK + ADD_COST;
                    char data[MESSAGE_TEXT_SIZE];
					strncpy( data, response.data, MESSAGE_TEXT_SIZE);
                    disk.add(data);
                }
                else if(response.mtype == DELETE_REQ)
                {
                    localCLK = CLK + DELETE_COST;
                    int index = int(response.data[0])-int('0');
                    disk.remove(index);
                }
            }
        }
    };
}




