#include "MasterController.h"
#include "NetworkCommandModule.h"


int main()
{
	MasterController main_controller;
	NetworkCommandModule network_conntroller(&main_controller);


	return 0;
}