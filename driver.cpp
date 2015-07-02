#include "MasterController.h"
#include "NetworkCommandModule.h"


int main()
{
	MasterController main_controller;
	NetworkCommandModule network_controller(&main_controller);
	network_controller.DoListen();
	//network_controller.LinearTest();

	while (1)
	{
	}
	return 0;
}