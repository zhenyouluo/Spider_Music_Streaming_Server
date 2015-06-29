#ifndef NETWORKCOMMANDMODULE_H
#define NETWORKCOMMANDMODULE_H


#include "MasterController.h"
#include  <winsock2.h>

int const DEFAULT_BUFFLEN = 32;


// Manage connection to clients. Listen to requests from clients for enqueueing tracks, skipping tracks,
// ect. Pass said requests onto the MasterController oject to do what it will with them.
class NetworkCommandModule
{
public:
	NetworkCommandModule(MasterController * dump_commands_here);
	void InitNetwork();
	void DoListen();
	~NetworkCommandModule();
private:
	// Give it a pointer to the MasterController object 
	MasterController * m_master_controller_ptr;

	// Buncha networking vars
    WSADATA m_wsaData;
	int m_network_result;
	int m_network_send_result;
	SOCKET m_listen_socket;
	SOCKET m_client_socket;
	char m_rec_buff[DEFAULT_BUFFLEN];
	int m_rec_buff_len;
};

#endif