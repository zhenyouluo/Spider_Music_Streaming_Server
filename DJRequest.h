#ifndef DJREQUEST_H
#define DJREQUEST_H

#include <string>

using std::string;

enum request_type {null_req, Pause, Play, Queue, Skip};

class DJReqeust
{
public:
	string m_client_id;
	enum request_type m_request_type;
	string m_request_details;
};

#endif