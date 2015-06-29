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
	bool QueueDJRequest( DJReqeust queue_this);
private:
	queue <DJReqeust> m_request_queue;
};



#endif