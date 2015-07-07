#ifndef NETWORKCOMMANDMODULE_H
#define NETWORKCOMMANDMODULE_H


#include "MasterController.h"



#include <process.h>


// Manage connection to clients. Listen to requests from clients for enqueueing tracks, skipping tracks,
// ect. Pass said requests onto the MasterController oject to do what it will with them.
class NetworkCommandModule
{
public:
	NetworkCommandModule(MasterController * dump_commands_here);
	int DoListen();  // Call after the server is created.
	~NetworkCommandModule();
	SOCKET GetCurClientSocket(); 
	void ClientThreadDone(int index_of_thread_done);


	// Public so client threads we spin up have access.
	HANDLE m_init_semaphore;  //When we spin up a client thread, the thread uses this semaphore
	// to signal that it is done reading our data initializing.
	HANDLE m_kill_semaphore; //Protects the flags that indicate when a client thread is done. 
	HANDLE m_client_thread_id_index[MAX_CLIENTS]; // an array of currently running thread handels.
	// Indexed by intigers 0-MAX_CLIENTS; 
	bool m_client_thread_done_index[MAX_CLIENTS]; // a parrallel array of flags to indicate what client threads
	// are done and need cleaning up now.
	int m_client_thread_index;
	int m_num_clients;

	MasterController * m_master_controller_ptr;

private:
	int InitNetwork(); // Called by CTOR

	bool m_kill_true; // If set to true, server cleans up and shuts down.
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