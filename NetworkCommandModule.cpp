#include "NetworkCommandModule.h"


DWORD WINAPI ServeClient(LPVOID lpParameter);
DWORD WINAPI CleanOldConnections(LPVOID lpParameter);

NetworkCommandModule::NetworkCommandModule(MasterController * dump_commands_here)
{
	m_master_controller_ptr = dump_commands_here;
	m_num_clients = 0;
	m_kill_true = false;
	InitNetwork();
}



NetworkCommandModule::~NetworkCommandModule()
{
	
	m_master_controller_ptr = nullptr;
}


int NetworkCommandModule::InitNetwork()
{
	m_addr_info_result = NULL;
	m_rec_buff_len = DEFAULT_BUFFLEN; 

	m_listen_socket = INVALID_SOCKET;
	m_client_socket = INVALID_SOCKET;
	
	m_init_semaphore = CreateSemaphore( NULL, 0, 1, NULL);
	m_kill_semaphore = CreateSemaphore( NULL, 1, 1, NULL);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_client_thread_done_index[i] = false;
		m_client_thread_id_index[i] = NULL;
	}
	

	m_network_result = WSAStartup(MAKEWORD(2,2), &m_wsaData);
	if (m_network_result != 0)
	{
		printf("WSAStartup failed.\n");
		return 1;
	}

	ZeroMemory(&m_addr_info_hints,  sizeof(m_addr_info_hints)); 
	m_addr_info_hints.ai_family = AF_INET;
	m_addr_info_hints.ai_socktype = SOCK_STREAM;
	m_addr_info_hints.ai_protocol = IPPROTO_TCP;
	m_addr_info_hints.ai_flags = AI_PASSIVE;

	// resolve the server address and port
	m_network_result = getaddrinfo(NULL, DEFUALT_PORT, &m_addr_info_hints, &m_addr_info_result);
	if (m_network_result != 0)
	{
		printf("Address resolution failed.\n");
		WSACleanup();
		return 1;
	}



	// Create the socket for the server.
	m_listen_socket = socket(m_addr_info_result->ai_family,
							 m_addr_info_result->ai_socktype,
							 m_addr_info_result->ai_protocol);
	if (m_listen_socket == INVALID_SOCKET)
	{
		printf("Invalid Socket.\n");
		freeaddrinfo(m_addr_info_result);
		WSACleanup();
		return 1;
	}


	// Bind the socket
	m_network_result = bind( m_listen_socket, 
							 m_addr_info_result->ai_addr, 
							(int)m_addr_info_result->ai_addrlen);
	if (m_network_result == SOCKET_ERROR)
	{
		printf("Failed to bind.\n");
		freeaddrinfo(m_addr_info_result);
		closesocket(m_listen_socket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(m_addr_info_result);


	return 0;
}



int NetworkCommandModule::DoListen()
{
	char deny_msg[12] = {"Denied"};
	HANDLE temp_handle;
	HANDLE cleanup_thread = CreateThread (NULL, 0, CleanOldConnections, (LPVOID) this, 0, NULL);

	// Receive
	do 
	{
		////////
		// Temporarily Force the master controller to handel requests.
		m_master_controller_ptr->HandelRequests();
		
		// listen
		m_network_result = listen(m_listen_socket, SOMAXCONN);
		if (m_network_result == SOCKET_ERROR)
		{
			printf("Listen Failed.\n");
			closesocket(m_listen_socket);
			WSACleanup();
			return 1;
		}


		// accept the client socket
		m_client_socket = accept(m_listen_socket, NULL, NULL);
		if (m_client_socket == INVALID_SOCKET)
		{
			printf("Accept failed.\n");
			closesocket(m_listen_socket);
			WSACleanup();
			return 1;
		}

		

		// Spin up a new thread for our client
		if (m_num_clients < MAX_CLIENTS)
		{			
			temp_handle = CreateThread (NULL, 0, ServeClient, (LPVOID) this, CREATE_SUSPENDED, NULL);
			m_client_thread_index = 0;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_client_thread_id_index[i] == NULL) // if you find a blank space in the index.
				{
					m_client_thread_id_index[i] = temp_handle; // assign the handle there.
					m_client_thread_index = i; // client thread will read this so it knows where to look later.
					i = MAX_CLIENTS; // done looking.
				}
			}
			// now start the thread, let it init.
			ResumeThread( m_client_thread_id_index[m_client_thread_index]);
			
			// Use a semaphore to ensure we don't change the init data until the client thread has read it.
				
			// Block until the child thread is properly initilized.
			WaitForSingleObject(m_init_semaphore, INFINITE);
			// semaphore signaled. Proceed. 
			m_num_clients++;
			

			// call CloseHandle when done with the thread.
			if (m_num_clients == MAX_CLIENTS)
				printf("Max Clients reached.");
		}
		else
		{
			printf("Max clients reached. New clients denied.");
			
			send(m_client_socket, deny_msg, (int)strlen(deny_msg), 0);
		}

		

	} while (m_kill_true == false);



	// You really should never get here unless there is an error, or some internal singal to shut down the server.
	closesocket(m_listen_socket);

	m_network_result = shutdown(m_client_socket, SD_SEND);
	if (m_network_result == SOCKET_ERROR)
	{
		printf("Accept failed.\n");
		closesocket(m_listen_socket);
		WSACleanup();
		return 1;
	}
	return 0;
}
SOCKET NetworkCommandModule::GetCurClientSocket()
{
	return m_client_socket;
}



/////////////////
// Thread job whose only job is to clean up old client threads when
// they signal that they are done. 
DWORD WINAPI CleanOldConnections(LPVOID lpParameter)
{
	NetworkCommandModule* ref_to_parent = (NetworkCommandModule*) lpParameter;
	SOCKET internal_socket = ref_to_parent->GetCurClientSocket(); 
	
	while (1)
	{
		// Check to see if any of our client threads are done.
		WaitForSingleObject(ref_to_parent->m_kill_semaphore, INFINITE);
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (ref_to_parent->m_client_thread_done_index[i] == true)
			{ // a certain client is done. 
				CloseHandle(ref_to_parent->m_client_thread_id_index[i]);
				ref_to_parent->m_client_thread_done_index[i] = false;
				ref_to_parent->m_client_thread_id_index[i] = NULL;
				ref_to_parent->m_num_clients--;      // Decouple num_clients and client index. 
			}

		}
		ReleaseSemaphore( ref_to_parent->m_kill_semaphore, 1, NULL);

	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////////////
// Thread function that manages communications with a single established client,
// Checks with the master controller for information to send to the client.
// Listens for packets from client, and dumps all packets recieved in 
// a queue in MasterController for further processing.
/////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServeClient(LPVOID lpParameter)
{	
	//ThreadMessage message_from_parent = (ThreadMessage) lpParameter;
	//SOCKET internal_socket = () lpParameter;
	NetworkCommandModule* ref_to_parent = (NetworkCommandModule*) lpParameter;
	SOCKET internal_socket = ref_to_parent->GetCurClientSocket(); 

	int this_threads_index = ref_to_parent->m_client_thread_index; // Where in the index we live.  
	int bytes_sent = 0;
	int bytes_recieved = 0;
	bool client_done = false;

	string send_buf= "A";
	char rec_buf[64]="";

	ReleaseSemaphore( ref_to_parent->m_init_semaphore, 1, NULL); // Tell big daddy we are done init.


	send_buf = "Testing server connection.\n";
	bytes_sent = send( internal_socket, send_buf.data(), send_buf.size(), 0);

	if (bytes_sent == SOCKET_ERROR)
	{
		printf( "Test broadcast failed.\n");
		client_done = true;
	}


	while (client_done == false)
	{
		// Check to see if we have stuff to send.
		if (ref_to_parent->m_master_controller_ptr->ClientChecksMail(this_threads_index) == true) 
		{		
			send_buf = ref_to_parent->m_master_controller_ptr->ClientReadsMail(this_threads_index);
					// Send bytes if we have something to send. 
			bytes_sent = send( internal_socket, send_buf.data(), send_buf.size(), 0);
			if (bytes_sent == SOCKET_ERROR)
			{
				printf( "Could not send message.\n");
				client_done = true;
			}
		}
		


		ZeroMemory(rec_buf, sizeof(rec_buf));
		bytes_recieved = recv(internal_socket, rec_buf, 32, MSG_PEEK); // Peek for new messages.
		if (bytes_recieved > 0) // If message. Processes. 
		{
			DJRequest req_for_you(this_threads_index, null_req, "no details");
			bytes_recieved = recv(internal_socket, rec_buf, 32, 0);  // Look into timeout func. 
			ref_to_parent->m_master_controller_ptr->QueueDJRequest(req_for_you); // Pass that information along.

			// Wait for some kind of response.


			send_buf = "A";
			send(internal_socket, send_buf.data(),  send_buf.size(), 0); // Send cmd_awk.

			printf("Client said: %s\n", rec_buf);

			if (strcmp(rec_buf, "kill") == 0)
			{
				client_done = true;
			}

			if (strcmp(rec_buf, "") == 0)
			{
				client_done = true;
			}
		}
		

	}

	printf("Client disconnected.\n");

	closesocket(internal_socket);
	ref_to_parent->ClientThreadDone(this_threads_index );

	return 1;
}

/////////////////////////////////////////////////////////////
//Client thread runs this when it is done and client has disconnected. 
//Signals the main threads that this thread is done and can be cleaned up.
/////////////////////////////////////////////////////////
void NetworkCommandModule::ClientThreadDone(int index_of_thread_done)
{
	WaitForSingleObject(m_kill_semaphore, INFINITE);
	m_client_thread_done_index[index_of_thread_done] = true;
	ReleaseSemaphore(m_kill_semaphore, 1, NULL);
};
