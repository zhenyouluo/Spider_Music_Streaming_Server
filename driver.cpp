#include "MasterController.h"
#include "NetworkCommandModule.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


int main()
{
	
	
	MasterController main_controller;
	NetworkCommandModule network_controller(&main_controller);
	network_controller.DoListen();
	

	while (1)
	{
	}

	_CrtDumpMemoryLeaks();
	return 0;
}