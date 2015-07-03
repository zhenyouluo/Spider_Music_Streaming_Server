#ifndef MASTERCONTROLLER_h
#define MASTERCONTROLLER_h


#include "DJRequest.h"
#include <queue>
using std::queue;

class MasterController
{
public:
	MasterController();
	~MasterController();
	bool QueueDJRequest( DJRequest queue_this);
	//HANDLE m_cmd_awknowledged_semaphore;

private:
	queue <DJRequest> m_request_queue;
};



#endif