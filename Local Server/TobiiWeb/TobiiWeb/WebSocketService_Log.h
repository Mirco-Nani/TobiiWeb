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

#include "WebSocket_Services.h"

class WebSocketApplicationResponseLogger : public ET_Consumer
{
public:
	WebSocketApplicationResponseLogger(ET_Producer<WebSocketSession_clientApplicationContent>* source)
		: ET_Consumer(source) {};
	virtual void OnReceive(WebSocketSession_content content);
	virtual void OnReceive(WebSocketSession_clientApplicationContent content);

};

class WebSocket_Log_Service : public WebSocket_Generic_Service
{
public:
	virtual string getName() { return "Log_Service"; };
	WebSocket_Log_Service();

	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	virtual void onSessionClosed(WebSocket_Session* session);
protected:
	virtual void stop_service();
};