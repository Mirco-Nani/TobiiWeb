#include "stdafx.h"
#include "TobiiEyeX_GlobalGazeTracker.h"
#include "ET_Interaction.h"
#include "utils.h"

#include <eyex\EyeX.h>
#include <stdio.h>
#include <iostream>
#pragma comment (lib, "Tobii.EyeX.Client.lib")

//Source for external comunication
ET_Producer<ET_GazeCoordinates_Content>* coordinatesSource;
ET_Producer<ET_Log>* logSource;

// ID of the global interactor that provides our data stream; must be unique within the application.
static const TX_STRING InteractorId = "global_gaze_webtracker";

// global variables
static TX_HANDLE g_hGlobalInteractorSnapshot = TX_EMPTY_HANDLE;

TX_CONTEXTHANDLE hContext = TX_EMPTY_HANDLE;
TX_TICKET hConnectionStateChangedTicket = TX_INVALID_TICKET;
TX_TICKET hEventHandlerTicket = TX_INVALID_TICKET;

void Log(std::string s)
{
	logSource->Emit(ET_Log(s));
}

/*
* Initializes g_hGlobalInteractorSnapshot with an interactor that has the Gaze Point behavior.
*/
unsigned int InitializeGlobalInteractorSnapshot(TX_CONTEXTHANDLE hContext, TX_GAZEPOINTDATAMODE filter)
{
	TX_HANDLE hInteractor = TX_EMPTY_HANDLE;
	TX_GAZEPOINTDATAPARAMS params = { filter };
	unsigned int success;

	success = txCreateGlobalInteractorSnapshot(
		hContext,
		InteractorId,
		&g_hGlobalInteractorSnapshot,
		&hInteractor) == TX_RESULT_OK;
	success &= txCreateGazePointDataBehavior(hInteractor, &params) == TX_RESULT_OK;

	txReleaseObject(&hInteractor);

	return success;
}

/*
* Callback function invoked when a snapshot has been committed.
*/
void TX_CALLCONVENTION OnSnapshotCommitted(TX_CONSTHANDLE hAsyncData, TX_USERPARAM param)
{
	// check the result code using an assertion.
	// this will catch validation errors and runtime errors in debug builds. in release builds it won't do anything.

	TX_RESULT result = TX_RESULT_UNKNOWN;
	txGetAsyncDataResultCode(hAsyncData, &result);
}

/*
* Callback function invoked when the status of the connection to the EyeX Engine has changed.
*/
void TX_CALLCONVENTION OnEngineConnectionStateChanged(TX_CONNECTIONSTATE connectionState, TX_USERPARAM userParam)
{
	switch (connectionState) {
	case TX_CONNECTIONSTATE_CONNECTED: {
		unsigned int success;
		Log("The connection state is now CONNECTED (We are connected to the EyeX Engine)");
		// commit the snapshot with the global interactor as soon as the connection to the engine is established.
		// (it cannot be done earlier because committing means "send to the engine".)
		success = txCommitSnapshotAsync(g_hGlobalInteractorSnapshot, OnSnapshotCommitted, NULL) == TX_RESULT_OK;
		if (!success) {
			Log("Failed to initialize the data stream.");
		}
		else {
			Log("Waiting for gaze data to start streaming...");
		}
	}
	break;

	case TX_CONNECTIONSTATE_DISCONNECTED:
		Log("The connection state is now DISCONNECTED (We are disconnected from the EyeX Engine)");
		break;

	case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
		Log("The connection state is now TRYINGTOCONNECT (We are trying to connect to the EyeX Engine)");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
		Log("The connection state is now SERVER_VERSION_TOO_LOW: this application requires a more recent version of the EyeX Engine to run.");
		break;

	case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
		Log("The connection state is now SERVER_VERSION_TOO_HIGH: this application requires an older version of the EyeX Engine to run.");
		break;
	}
}

/*
* Handles an event from the Gaze Point data stream.
*/
void OnGazeDataEvent(TX_HANDLE hGazeDataBehavior)
{
	TX_GAZEPOINTDATAEVENTPARAMS eventParams;
	if (txGetGazePointDataEventParams(hGazeDataBehavior, &eventParams) == TX_RESULT_OK) {
		coordinatesSource->Emit(ET_GazeCoordinates_Content(eventParams.X, eventParams.Y, getCurrentTimestamp() ));
	}
	else {
		Log("Failed to interpret gaze data event packet.");
	}
}

/*
* Callback function invoked when an event has been received from the EyeX Engine.
*/
void TX_CALLCONVENTION HandleEvent(TX_CONSTHANDLE hAsyncData, TX_USERPARAM userParam)
{
	TX_HANDLE hEvent = TX_EMPTY_HANDLE;
	TX_HANDLE hBehavior = TX_EMPTY_HANDLE;

	txGetAsyncDataContent(hAsyncData, &hEvent);

	// Uncomment the following line of code to view the event object. The same function can be used with any interaction object.
	//OutputDebugStringA(txDebugObject(hEvent));

	if (txGetEventBehavior(hEvent, &hBehavior, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK) {
		OnGazeDataEvent(hBehavior);
		txReleaseObject(&hBehavior);
	}

	// since this is a very simple application with a single interactor and a single data stream, 
	// the event handling code can be very simple too. A more complex application would typically have to 
	// check for multiple behaviors and route events based on interactor IDs.

	txReleaseObject(&hEvent);
}


void start_TobiiEyeX_GlobalGazeTracker(TX_GAZEPOINTDATAMODE filter)
{
	unsigned int success;

	// initialize and enable the context that is our link to the EyeX Engine.
	success = txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, NULL, NULL, NULL, NULL) == TX_RESULT_OK;
	success &= txCreateContext(&hContext, TX_FALSE) == TX_RESULT_OK;
	success &= InitializeGlobalInteractorSnapshot(hContext, filter);
	success &= txRegisterConnectionStateChangedHandler(hContext, &hConnectionStateChangedTicket, OnEngineConnectionStateChanged, NULL) == TX_RESULT_OK;
	success &= txRegisterEventHandler(hContext, &hEventHandlerTicket, HandleEvent, NULL) == TX_RESULT_OK;
	success &= txEnableConnection(hContext) == TX_RESULT_OK;

	if (success) {
		Log("Initialization was successful.");
	}
	else {
		Log("Initialization failed.");
	}
}

void stop_TobiiEyeX_GlobalGazeTracker()
{
	unsigned int success;

	// disable and delete the context.
	txDisableConnection(hContext);
	txReleaseObject(&g_hGlobalInteractorSnapshot);
	success = txShutdownContext(hContext, TX_CLEANUPTIMEOUT_DEFAULT, TX_FALSE) == TX_RESULT_OK;
	success &= txReleaseContext(&hContext) == TX_RESULT_OK;
	success &= txUninitializeEyeX() == TX_RESULT_OK;
	if (!success) {
		Log("EyeX could not be shut down cleanly. Did you remember to release all handles?");
	}
}



TobiiEyeX_GlobalGazeTracker* TobiiEyeX_GlobalGazeTracker::instance = 0;
unsigned int TobiiEyeX_GlobalGazeTracker::running = 0;
unsigned int TobiiEyeX_GlobalGazeTracker::ready = 0;
TobiiEyeX_GlobalGazeTracker* TobiiEyeX_GlobalGazeTracker::Instance()

{
	if (!instance)
		instance = new TobiiEyeX_GlobalGazeTracker;
	return instance;
}


void TobiiEyeX_GlobalGazeTracker::Init(ET_Producer<ET_GazeCoordinates_Content>* coordsSource, ET_Producer<ET_Log>* extLogSource)
{
	coordinatesSource = coordsSource;
	logSource = extLogSource;
	ready = 1;
}

unsigned int TobiiEyeX_GlobalGazeTracker::start(TX_GAZEPOINTDATAMODE filter)
{
	if (!ready)
	{
		Log("ERROR: TobiiEyeX_GlobalGazeTracker is not ready, did you forget to call Init first?");
		
		return -1;
	}

	if (!running)
	{
		start_TobiiEyeX_GlobalGazeTracker(filter);
		running = 1;
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int TobiiEyeX_GlobalGazeTracker::stop()
{
	if (running)
	{
		stop_TobiiEyeX_GlobalGazeTracker();
		return 1;
	}
	else
	{
		return 0;
	}
}

TobiiEyeX_GlobalGazeTracker::TobiiEyeX_GlobalGazeTracker(){}

TobiiEyeX_GlobalGazeTracker::~TobiiEyeX_GlobalGazeTracker()
{
	stop();
	instance = 0;
	ready = 0;
}
