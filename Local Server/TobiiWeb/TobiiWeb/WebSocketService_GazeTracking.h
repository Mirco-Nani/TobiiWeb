/*
*
*  Copyright 2016 Mirco Nani
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*/

#pragma once
#include <map>
#include <vector>

#include "WebSocket_Services.h"
#include "TobiiEyeX_GlobalGazeTracker.h"
#include "Windows_sensing.h"

class WindowInfo_to_WebSocketSession_contentTranslator : public ET_Consumer
{
public:
	WindowInfo_to_WebSocketSession_contentTranslator(ET_Producer<ET_WindowInfo_Content>* source, ET_Producer<WebSocketSession_content>* producer)
		: _producer(producer), ET_Consumer(source) {};
	virtual void OnReceive(ET_WindowInfo_Content content);

	virtual void setProducer(ET_Producer<WebSocketSession_content>* producer);
	virtual ET_Producer<WebSocketSession_content>* getProducer();
private:
	ET_Producer<WebSocketSession_content>* _producer = nullptr;
};


class WebSocket_GazeTracking_Service : public WebSocket_Generic_Service
{
public:
	virtual string getName() { return "GazeTracking_Service"; };
	WebSocket_GazeTracking_Service(int log_level = 0);
	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	//virtual ET_Producer<WebSocketSession_content>* get_WebSocketSessionContent_Producer();

protected:
	virtual void start_service();
	virtual void stop_service();

	unsigned int _logLevel;
	TobiiEyeX_GlobalGazeTracker* _gazeTracker;
	ET_Producer<ET_GazeCoordinates_Content>* _coordinatesSource;
	WindowPerceiver* _windowPerceiver;
	ET_Producer<ET_WindowInfo_Content>* _windowInfoSource;
	WindowInfo_to_WebSocketSession_contentTranslator* _translator;
	ET_Producer<WebSocketSession_content>* _websocket_producer;

	//log level 1
	ET_Producer<ET_Log>* _gazeTracker_logSource;
	ET_Logger* _eyeX_logDestination;

	// log level 2
	ET_Logger* _coordinatesLogger;
	ET_Logger* _windowInfoDestination;
};
