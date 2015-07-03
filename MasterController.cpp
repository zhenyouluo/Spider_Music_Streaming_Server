#include "MasterController.h"



MasterController::MasterController()
{
	m_request_queue = queue<DJRequest>(); 
}
MasterController::~MasterController()
{
	m_request_queue.empty();
}

bool MasterController::QueueDJRequest( DJRequest queue_this)
{
	 m_request_queue.push(queue_this);
	return true;
}