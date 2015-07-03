#include "NetworkCommandModule.h"


DWORD WINAPI ServeClient(LPVOID lpParameter);

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
	char m_rec_buff [DEFAULT_BUFFLEN];
	m_rec_buff_len = DEFAULT_BUFFLEN; 

	m_listen_socket = INVALID_SOCKET;
	m_client_socket = INVALID_SOCKET;
	
	m_init_semaphore = CreateSemaphore( NULL, 0, 1, NULL);
	m_kill_semaphore = CreateSemaphore( NULL, 1, 1, NULL);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_client_thread_done_index[i] = false;
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


	
}



int NetworkCommandModule::DoListen()
{
	char deny_msg[12] = {"Denied"};
	// Receive
	do 
	{
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
			
			m_client_thread_id_index[m_num_clients] = CreateThread (NULL, 0, ServeClient, (LPVOID) this, 0, NULL);
			// Use a semaphore to ensure we don't change the client socket until the serverclient thread has read it.
				
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

		// Check to see if any of our client threads are done.
		WaitForSingleObject(m_kill_semaphore, INFINITE);
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_client_thread_done_index[i] == true)
			{ // a certain client is done. 
				CloseHandle(m_client_thread_id_index[i]);
				m_client_thread_done_index[i] == false;
			}

		}
		ReleaseSemaphore( m_kill_semaphore, 1, NULL);


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
}
SOCKET NetworkCommandModule::GetCurClientSocket()
{
	return m_client_socket;
}

DWORD WINAPI ServeClient(LPVOID lpParameter)
{	
	//ThreadMessage message_from_parent = (ThreadMessage) lpParameter;
	//SOCKET internal_socket = () lpParameter;
	NetworkCommandModule* ref_to_parent = (NetworkCommandModule*) lpParameter;
	SOCKET internal_socket = ref_to_parent->GetCurClientSocket(); 

	int this_threads_index = ref_to_parent->m_num_clients; 
	int bytes_sent = 0;
	int bytes_recieved = 0;
	bool client_done = false;

	char send_buf[64]="";
	char rec_buf[64]="";

	char timebuf[64];
	ReleaseSemaphore( ref_to_parent->m_init_semaphore, 1, NULL);


	sprintf(send_buf, "Testing server connection.\n");
	bytes_sent = send( internal_socket, send_buf, strlen(send_buf), 0);

	if (bytes_sent == SOCKET_ERROR)
	{
		printf( "Test broadcast failed.\n");
		client_done = true;
	}


	while (client_done == false)
	{
		ZeroMemory(rec_buf, sizeof(rec_buf));

		bytes_recieved = recv(internal_socket, rec_buf, 32, 0);  // Look into timeout func. 



		printf("Client said: %s\n", rec_buf);

		if (strcmp(rec_buf, "kill") == 0)
		{
			client_done = true;
		}

		if (rec_buf == NULL)
		{
			client_done = true;
		}

	}

	printf("Client disconnected.\n");

	closesocket(internal_socket);
	ref_to_parent->ClientThreadDone(this_threads_index );

	return 1;
}

void NetworkCommandModule::ClientThreadDone(int index_of_thread_done)
{
	WaitForSingleObject(m_kill_semaphore, INFINITE);
	m_client_thread_done_index[index_of_thread_done] = true;
	m_num_clients--;
	ReleaseSemaphore(m_kill_semaphore, 1, NULL);
};

void NetworkCommandModule::DecrNumClients()
{
	if (m_num_clients > 0)
		 m_num_clients--;

}