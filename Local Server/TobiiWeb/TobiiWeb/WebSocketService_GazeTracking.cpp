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

#include "stdafx.h"
#include "WebSocketService_GazeTracking.h"

void WindowInfo_to_WebSocketSession_contentTranslator::setProducer(ET_Producer<WebSocketSession_content>* producer)
{
	_producer = producer;
}

ET_Producer<WebSocketSession_content>* WindowInfo_to_WebSocketSession_contentTranslator::getProducer()
{
	return _producer;
}

void WindowInfo_to_WebSocketSession_contentTranslator::OnReceive(ET_WindowInfo_Content content)
{
	if (_producer != nullptr)
	{
		char json[FRAME_LENGHT];
		double x = content.global_gaze_x - content.window_x;
		double y = content.global_gaze_y - content.window_y;
		double timestamp = content.global_gaze_timestamp;
		int len = sprintf_s(json, "{\"type\":\"gaze_coordinates\",\"x\":%f,\"y\":%f,\"timestamp\":%f,\"global_x\":%f,\"global_y\":%f,\"viewport_x\":%f,\"viewport_y\":%f}",
			x, y, timestamp, content.global_gaze_x, content.global_gaze_y, content.window_x, content.window_y);
		_producer->Emit(WebSocketSession_content(json, len, content.window_hwnd, content.ancestorWindow_hwnd));
	}
}

WebSocket_GazeTracking_Service::WebSocket_GazeTracking_Service(int log_level)
{
	_logLevel = log_level;
	_gazeTracker = TobiiEyeX_GlobalGazeTracker::Instance();
	_gazeTracker_logSource = new ET_Producer<ET_Log>();

	_coordinatesSource = new ET_Producer<ET_GazeCoordinates_Content>();
	_windowPerceiver = new WindowPerceiver(_coordinatesSource);
	_windowInfoSource = new ET_Producer<ET_WindowInfo_Content>();

	_websocket_producer = new ET_Producer<WebSocketSession_content>();
	_translator = new WindowInfo_to_WebSocketSession_contentTranslator(_windowInfoSource, _websocket_producer);


	_windowPerceiver->Init(_windowInfoSource);
	_gazeTracker->Init(_coordinatesSource, _gazeTracker_logSource);

	if (_logLevel >= 1)
	{
		_eyeX_logDestination = new ET_Logger(_gazeTracker_logSource);
	}
	if (_logLevel >= 2)
	{
		_coordinatesLogger = new ET_Logger(_coordinatesSource);
		_windowInfoDestination = new ET_Logger(_windowInfoSource);
	}
}
ET_Producer<WebSocketSession_content>* WebSocket_GazeTracking_Service::onNewSubscription(WebSocket_Session* session)
{
	return _websocket_producer;
}

void WebSocket_GazeTracking_Service::start_service()
{
	_gazeTracker->start(TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED);
}

void WebSocket_GazeTracking_Service::stop_service()
{
	_gazeTracker->stop();
}