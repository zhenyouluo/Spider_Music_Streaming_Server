#include "NetworkCommandModule.h"


NetworkCommandModule::NetworkCommandModule(MasterController * dump_commands_here)
{
	m_master_controller_ptr = dump_commands_here;
}



NetworkCommandModule::~NetworkCommandModule()
{
	
	m_master_controller_ptr = nullptr;
}


void NetworkCommandModule::InitNetwork()
{

	m_network_result = WSAStartup(MAKEWORD(2,2), &m_wsaData);
	m_listen_socket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
	m_rec_buff_len = DEFAULT_BUFFLEN; 
}

void NetworkCommandModule::DoListen()
{

}