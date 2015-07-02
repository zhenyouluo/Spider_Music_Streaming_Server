#include "NetworkCommandModule.h"


DWORD WINAPI ServeClient(LPVOID lpParameter);

NetworkCommandModule::NetworkCommandModule(MasterController * dump_commands_here)
{
	m_master_controller_ptr = dump_commands_here;
	m_num_clients = 0;
	m_max_clients = 16;
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


int NetworkCommandModule::LinearTest()
{
	m_network_result = listen(m_listen_socket, SOMAXCONN);
	if (m_network_result == SOCKET_ERROR)
	{
		printf("Listen Failed.\n");
		closesocket(m_listen_socket);
		WSACleanup();
		return 1;
	}


	m_client_socket = accept(m_listen_socket, NULL, NULL);
	if (m_client_socket == INVALID_SOCKET)
	{
		printf("Accept failed.\n");
		closesocket(m_listen_socket);
		WSACleanup();
		return 1;
	}
	DWORD client_thread_id_temp;
	CreateThread (NULL, 0, ServeClient, (LPVOID) m_client_socket, 0, &client_thread_id_temp);
	return 0;
}

int NetworkCommandModule::DoListen()
{
	DWORD client_thread_id[32]; // Magic numbers. Fix this.

	// Receive
	do 
	{
		// listen
		m_network_result = listen(m_listen_socket, SOMAXCONN);
		if (m_network_result == SOCKET_ERROR)
		{
			//printf("Listen Failed.\n");
			//closesocket(m_listen_socket);
			//WSACleanup();
			//return 1;
		}
		//else
		//{

			// accept the client socket
			m_client_socket = accept(m_listen_socket, NULL, NULL);
			if (m_client_socket == INVALID_SOCKET)
			{
				printf("Accept failed.\n");
				closesocket(m_listen_socket);
				WSACleanup();
				return 1;
			}

			//closesocket(m_listen_socket);

			// Spin up a new thread for our client
			if (m_num_clients < m_max_clients)
			{			
				CreateThread (NULL, 0, ServeClient, (LPVOID) m_client_socket, 0, &client_thread_id[m_num_clients]);
				m_num_clients++;
			}
			else
			{
				printf("Max clients reached. New clients denied.");

			}
		//}

	} while (m_kill_true == false);


	m_network_result = shutdown(m_client_socket, SD_SEND);
	if (m_network_result == SOCKET_ERROR)
	{
		printf("Accept failed.\n");
		closesocket(m_listen_socket);
		WSACleanup();
		return 1;
	}
}

DWORD WINAPI ServeClient(LPVOID lpParameter)
{	
	SOCKET internal_socket = (SOCKET) lpParameter;

	int bytes_sent;
	int bytes_recieved;
	bool client_done = false;

	char send_buf[64]="";
	char rec_buf[64]="";

	char timebuf[64];

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

		bytes_recieved = recv(internal_socket, rec_buf, 32, 0);
		printf("Client said: %s\n", rec_buf);

		if (strcmp(rec_buf, "kill") == 0)
		{
			client_done = true;
		}

	}

	printf("Client disconnected.\n");

	closesocket(internal_socket);
	return 1;
}