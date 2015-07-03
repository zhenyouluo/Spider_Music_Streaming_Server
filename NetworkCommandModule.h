#ifndef NETWORKCOMMANDMODULE_H
#define NETWORKCOMMANDMODULE_H


#include "MasterController.h"
#include  <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

const int DEFAULT_BUFFLEN = 32;
const PCSTR DEFUALT_PORT = "27015";
//const int MAX_CLIENTS = 30;
const int MAX_CLIENTS = 5;

#include <process.h>


// Manage connection to clients. Listen to requests from clients for enqueueing tracks, skipping tracks,
// ect. Pass said requests onto the MasterController oject to do what it will with them.
class NetworkCommandModule
{
public:
	NetworkCommandModule(MasterController * dump_commands_here);
	int InitNetwork();
	int DoListen();
	~NetworkCommandModule();
	SOCKET GetCurClientSocket(); 
	void DecrNumClients();
	void ClientThreadDone(int index_of_thread_done);

	HANDLE m_init_semaphore;
	HANDLE m_kill_semaphore;
	HANDLE m_client_thread_id_index[MAX_CLIENTS];
	bool m_client_thread_done_index[MAX_CLIENTS];
	int m_num_clients;
private:
	// Give it a pointer to the MasterController object 
	MasterController * m_master_controller_ptr;

	bool m_kill_true;

	// Buncha networking vars
    WSADATA m_wsaData;
	int m_network_result;
	int m_network_send_result;
	SOCKET m_listen_socket;
	SOCKET m_client_socket;
	struct addrinfo *m_addr_info_result; 
	struct addrinfo m_addr_info_hints;
	int m_rec_buff_len;
	///////
	
	////////



};



#endif