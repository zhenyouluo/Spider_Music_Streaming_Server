#ifndef MASTERCONTROLLER_h
#define MASTERCONTROLLER_h

#include <process.h>

#include "DJRequest.h"
#include <queue>

#include  <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

const int DEFAULT_BUFFLEN = 32;
const PCSTR DEFUALT_PORT = "27015";
//const int MAX_CLIENTS = 30;
const int MAX_CLIENTS = 5;


using std::pair;
using std::queue;
using std::string;

class MasterController
{
public:
	MasterController();
	~MasterController();
	bool QueueDJRequest( DJRequest queue_this);
	int HandelRequests();
	bool ClientChecksMail(int client_id);
	string ClientReadsMail(int client_id);


private:
	pair <bool, string> m_data_out_array[MAX_CLIENTS];
	HANDLE m_data_send_semaphore;
	queue <DJRequest> m_request_queue;
	int SendClientMail( int client_id, string message_to_send);
};



#endif