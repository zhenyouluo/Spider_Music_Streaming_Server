#include "MasterController.h"




MasterController::MasterController()
{
	m_request_queue = queue<DJReqeust>(); 
}
MasterController::~MasterController()
{
	m_request_queue.empty();
}

bool MasterController::QueueDJRequest( DJReqeust queue_this)
{
	 m_request_queue.push(queue_this);
	return true;
}