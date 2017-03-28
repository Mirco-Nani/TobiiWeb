#include "stdafx.h"
#include <stdio.h>
#include "TobiiEyeX_GlobalGazeTracker.h"
#include "ET_Interaction.h"
#include "Windows_sensing.h"
#include "POCO_WebSocketServer.h"
#include "Utils.h"
#include "Screenshot.h"
#include "WebSocket_Services.h"
#include "POCO_HttpClient.h"
#include "POCO_FtpClient.h"

void test_POCO_WebSocketServer()
{
	POCO_WebSocketServer* webSocketServer = POCO_WebSocketServer::Instance();
	webSocketServer->start();
	printf("press any key to exit\n");
	getchar();
	printf("exiting...");
	webSocketServer->stop();
};

void test_TobiiEyeX_GlobalGazeTracker()
{
	//-------------------------------------------- System assemble --------------------------------------------
	TobiiEyeX_GlobalGazeTracker* gaze_tracker = TobiiEyeX_GlobalGazeTracker::Instance();

	ET_Producer<ET_Log>* eyeX_logSource = new ET_Producer<ET_Log>();
	//ET_Consumer* eyeX_logDestination = new ET_Consumer(eyeX_logSource);
	ET_Logger* eyeX_logDestination = new ET_Logger(eyeX_logSource);
	//inheritance test:
	//ET_Alternate_Consumer *logDestination = new ET_Alternate_Consumer(logSource);

	ET_Producer<ET_GazeCoordinates_Content>* coordinatesSource = new ET_Producer<ET_GazeCoordinates_Content>();
	//ET_Consumer *coordinatesDestination = new ET_Consumer(coordinatesSource);
	//inheritance test:
	//ET_Alternate_Consumer *coordinatesDestination = new ET_Alternate_Consumer(coordinatesSource);
	WindowPerceiver* windowPerceiver = new WindowPerceiver(coordinatesSource);
	ET_Producer<ET_WindowInfo_Content>* windowInfoSource = new ET_Producer<ET_WindowInfo_Content>();
	//ET_Consumer* windowInfoDestination = new ET_Consumer(windowInfoSource); //TO PRINT WINDOW STUFF
	windowPerceiver->Init(windowInfoSource);
	
	ET_Producer<ET_JSON_Content>* webSocket_applicationResponse_source = new ET_Producer<ET_JSON_Content>();
	//To log application-produced responses
	//POCO_WebSocket_Application_Response_Logger* app_response_logger = new POCO_WebSocket_Application_Response_Logger(webSocket_applicationResponse_source);

	ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
	ET_Consumer* webSocketServer_logDestination = new ET_Consumer(webSocketServer_logSource);
	

	POCO_WebSocketServer* webSocketServer = POCO_WebSocketServer::Instance();

	//--------------------------------------------- System Set-up --------------------------------------------
	gaze_tracker->Init(coordinatesSource, eyeX_logSource);
	
	webSocketServer->Init(windowInfoSource, webSocket_applicationResponse_source, webSocketServer_logSource);

	webSocketServer->start();

	

	gaze_tracker->start(TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED);



	// let the events flow until a key is pressed.
	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	//-------------------------------------------- System Tear-down --------------------------------------------
	gaze_tracker->stop();
	webSocketServer->stop();
}
/*
void test_ET_Interaction()
{
	ET_Producer *source= new ET_Producer();
	ET_Consumer *dest1 = new ET_Consumer(source);
	ET_Consumer *dest2 = new ET_Consumer(source);

	//source->AddDestination(dest1);
	//source->AddDestination(dest2);

	source->Emit<ET_GazeCoordinates_Content>(ET_GazeCoordinates_Content(1,1,1));

	printf("removing a consumer...\n");

	dest1->Detach();

	source->Emit<ET_Content>(ET_Content());
}
*/

void test_screenshot()
{
	unsigned long hwnd = GetHwndFrom::ScreenPoint(50, 50);
	ScreenshotPrinter::TakeScreenshotOfWindow(hwnd, "../media/screen.bmp");
}

void periodic_function()
{
	cout << "called" << endl;
}
void test_periodic_function()
{
	PeriodicFunction* Pf = new PeriodicFunction();
	Pf->start(periodic_function, 1000);

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	Pf->stop();
}

class Test_PeriodicLogger : public ET_Periodic_Producer<ET_Log>
{
protected:
	ET_Log Produce_Content()
	{
		return ET_Log("log happened");
	};
};
/*
void test_periodic_producer()
{
	Test_PeriodicLogger* source = new Test_PeriodicLogger();
	ET_Consumer* dest = new ET_Consumer(source);
	source->start();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	source->stop();
}*/
void test_gazeTrackingService()
{
	WebSocket_GazeTracking_Service* service = new WebSocket_GazeTracking_Service(2);
	service->start();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	service->stop();
}

//SERVICES

void test_serviceRegistration()
{
	POCO_WebSocketServer* server = POCO_WebSocketServer::Instance();

	ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
	ET_Consumer* webSocketServer_logDestination = new ET_Consumer(webSocketServer_logSource);

	server->Init(webSocketServer_logSource);

	WebSocket_GazeTracking_Service* gazeTracking_service = new WebSocket_GazeTracking_Service(1);
	server->RegisterService(gazeTracking_service);

	WebSocket_Log_Service* log_service = new WebSocket_Log_Service();
	server->RegisterService(log_service);

	server->start();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	server->stop();

	printf("Press any key to close the console\n");
	getchar();
	printf("Bye Bye.\n");
}

void test_serviceRegistration02()
{
	POCO_WebSocketServer* server = POCO_WebSocketServer::Instance();

	ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
	ET_Consumer* webSocketServer_logDestination = new ET_Consumer(webSocketServer_logSource);

	server->Init(webSocketServer_logSource);

	WebSocket_GazeTracking_Service* gazeTracking_service = new WebSocket_GazeTracking_Service(1);
	server->RegisterService(gazeTracking_service);

	WebSocket_Log_Service* log_service = new WebSocket_Log_Service();
	server->RegisterService(log_service);


	WebSocket_ClientData_Service* clientData_service = new WebSocket_ClientData_Service();
	server->RegisterService(clientData_service);

	ScreenshotFile_Service* screenshot_service = new ScreenshotFile_Service();
	server->RegisterService(screenshot_service);

	server->start();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	server->stop();

	printf("Press any key to close the console\n");
	getchar();
	printf("Bye Bye.\n");
}

void test_periodic_screenshots()
{
	ET_Producer<ScreenshotMetadata_content>* screenshotMetadata_source = new ET_Producer<ScreenshotMetadata_content>();
	ScreenshotMetadata_Logger* metadata_logger = new ScreenshotMetadata_Logger(screenshotMetadata_source);

	unsigned int hwnd = GetHwndFrom::ScreenPoint(0, 0);

	printf("%d\n", hwnd);

	Periodic_WindowScreenshotRequestor* periodic_screenshot_taker = new Periodic_WindowScreenshotRequestor(hwnd,"test");
	ET_Producer<ScreenshotRequest_content>* requester = periodic_screenshot_taker->getProducer();
	ScreenshotTaker* screensTaker = new ScreenshotTaker(requester, screenshotMetadata_source);

	periodic_screenshot_taker->start(2000);

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	periodic_screenshot_taker->stop();

}

void test_screenshots()
{
	ET_Producer<ScreenshotMetadata_content>* screenshotMetadata_source = new ET_Producer<ScreenshotMetadata_content>();
	ScreenshotMetadata_Logger* metadata_logger = new ScreenshotMetadata_Logger(screenshotMetadata_source);

	unsigned int hwnd = GetHwndFrom::ScreenPoint(0, 0);

	printf("%d\n", hwnd);

	//Periodic_WindowScreenshotRequestor* periodic_screenshot_taker = new Periodic_WindowScreenshotRequestor(hwnd, "test");
	ET_Producer<ScreenshotRequest_content>* requester = new ET_Producer<ScreenshotRequest_content>();//periodic_screenshot_taker->getProducer();
	ScreenshotTaker* screensTaker = new ScreenshotTaker(requester, screenshotMetadata_source);

	requester->Emit(ScreenshotRequest_content(hwnd,"../media/test.bmp"));

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");


}

void test_httpRequest()
{
	//SimpleHttpRequestTest();
	//SimpleHttpRequestTest_POST();
	//string response = POCO_HttpClient::POST_request("http://localhost:8888/simpleservlet2?username=user&pass=pass").response;
	//string response = POCO_HttpClient::GET_request("http://localhost:8888/simpleservlet2?username=user&pass=pass").response;

	

	//string response = POCO_HttpClient::POST_request("http://1-dot-mir-project.appspot.com/gaze_analytics?username=user&pass=pass").response;

	string response = POCO_HttpClient::GET_request("http://1-dot-mir-project.appspot.com/").response;

	cout << response << endl;

	printf("Press any key to exit...\n");
	getchar();

	printf("Exiting.\n");

}

void test_ftpUpload()
{
	POCO_FtpClient::test();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");
}

void test_HTTP_UNIQUE_ID()
{
	UniqueID::Init(new UniqueID_HTTPgetter());
	cout << UniqueID::get() << endl;

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");
}

void test_serviceRegistration03() {
	POCO_WebSocketServer* server = POCO_WebSocketServer::Instance();

	ET_Producer<ET_Log>* webSocketServer_logSource = new ET_Producer<ET_Log>();
	ET_Consumer* webSocketServer_logDestination = new ET_Consumer(webSocketServer_logSource);

	server->Init(webSocketServer_logSource);

	WebSocket_GazeTracking_Service* gazeTracking_service = new WebSocket_GazeTracking_Service(1);
	server->RegisterService(gazeTracking_service);

	WebSocket_Log_Service* log_service = new WebSocket_Log_Service();
	server->RegisterService(log_service);

	
	Echo_Service* echo_service = new Echo_Service();
	server->RegisterService(echo_service);
	
	Screenshot_Service* screenshot_service = new Screenshot_Service();
	server->RegisterService(screenshot_service);

	/*
	WebSocket_ClientData_Service* clientData_service = new WebSocket_ClientData_Service();
	server->RegisterService(clientData_service);

	ScreenshotFile_Service* screenshot_service = new ScreenshotFile_Service();
	server->RegisterService(screenshot_service);
	*/

	server->start();

	printf("Press any key to exit...\n");
	getchar();
	printf("Exiting.\n");

	server->stop();

	printf("Press any key to close the console\n");
	getchar();
	printf("Bye Bye.\n");
}

void test_base64Img()
{
	//string b64 = FileImage_to_Base64("../media/6598306717237248_1.jpg");
	string b64 = File_to_Base64("../media/0_1.jpg");
	//string b64 = FileImage_to_Base64("../media/longfile.txt");
	cout << b64 << endl;
	getchar();
}

/*
* Application entry point.
*/
int main(int argc, char* argv[])
{
	//test_TobiiEyeX_GlobalGazeTracker();
	//test_screenshot();
	//test_periodic_function();
	//test_periodic_producer();
	//test_gazeTrackingService();
	//test_serviceRegistration();
	//test_periodic_screenshots();
	//test_screenshots();

	//test_serviceRegistration02();
	//test_httpRequest();
	//test_ftpUpload();
	//test_HTTP_UNIQUE_ID();

	//test_ET_Interaction();

	//test_POCO_WebSocketServer();

	test_serviceRegistration03();
	//test_base64Img();

	return 0;
}