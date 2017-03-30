#include "stdafx.h"
#include <stdio.h>
#include "POCO_WebSocketServer.h"
#include "WebSocket_Services.h"
#include "WebSocketService_GazeTracking.h"
#include "WebSocketService_Log.h"
#include "WebSocketService_Echo.h"
#include "WebSocketService_Screenshot.h"

void start_system() {
	POCO_WebSocketServer* server = POCO_WebSocketServer::Instance();

	ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
	ET_Logger* webSocketServer_logDestination = new ET_Logger(webSocketServer_logSource);

	server->Init(webSocketServer_logSource);

	WebSocket_GazeTracking_Service* gazeTracking_service = new WebSocket_GazeTracking_Service(1);
	server->RegisterService(gazeTracking_service);

	WebSocket_Log_Service* log_service = new WebSocket_Log_Service();
	server->RegisterService(log_service);

	Echo_Service* echo_service = new Echo_Service();
	server->RegisterService(echo_service);

	Screenshot_Service* screenshot_service = new Screenshot_Service();
	server->RegisterService(screenshot_service);

	server->start();

	printf("Press any key to exit...\n\n");
	getchar();
	printf("Exiting.\n");

	server->stop();

	printf("Press any key to close the console\n");
	getchar();
	printf("Bye Bye.\n");
}

/*
* Application entry point.
*/
int main(int argc, char* argv[])
{
	start_system();
	return 0;
}