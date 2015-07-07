#include "MasterController.h"



MasterController::MasterController()
{
	m_request_queue = queue<DJRequest>(); 
	for (int i = 0; i > MAX_CLIENTS; i++)
	{
		m_data_out_array[i].first = false;
	}
	
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

int MasterController::HandelRequests()
{
	string temp_message = "You are client two.";
	// For now, Send the second client a message every time they send us anything.
	while (m_request_queue.empty() == false)
	{
		if (m_request_queue.front().m_client_id == 1)
		{
			SendClientMail(1, temp_message );
		}
			
			
			
		m_request_queue.pop();
	}
	return 0;
}

int MasterController::SendClientMail( int client_id, string message_to_send)
{
	if (client_id < MAX_CLIENTS)
	{
		WaitForSingleObject(m_data_send_semaphore, INFINITE);
		m_data_out_array[client_id].second = message_to_send;
		m_data_out_array[client_id].first = true;
		ReleaseSemaphore(m_data_send_semaphore, 1, NULL);
		return 0;
	}
	else
		return 1;
}


bool MasterController::ClientChecksMail(int client_id)
{
	return m_data_out_array[client_id].first;
}

string MasterController::ClientReadsMail(int client_id)
{
	string return_this = "NULL_MAIL";
	if ( m_data_out_array[client_id].first == true)
	{
		WaitForSingleObject(m_data_send_semaphore, INFINITE);
		return_this = m_data_out_array[client_id].second;
		m_data_out_array[client_id].first = false;
		ReleaseSemaphore( m_data_send_semaphore, 1, NULL);
	}
	return return_this;
}