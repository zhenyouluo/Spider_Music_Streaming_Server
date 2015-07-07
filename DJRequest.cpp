#include "DJRequest.h"

DJRequest::DJRequest(int client_id, enum request_type request_type, string request_details)
{
	m_client_id = client_id;
	m_request_type = request_type;
	m_request_details = request_details;

}

DJRequest::~DJRequest()
{
	m_client_id = 0;
	m_request_type = null_req;
	m_request_details = "";

}