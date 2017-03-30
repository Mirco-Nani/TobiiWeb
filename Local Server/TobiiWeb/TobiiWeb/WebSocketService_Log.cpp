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
#include "WebSocketService_Log.h"

WebSocket_Log_Service::WebSocket_Log_Service()
{

}

ET_Producer<WebSocketSession_content>* WebSocket_Log_Service::onNewSubscription(WebSocket_Session * session)
{
	_application_response_producers[session] = session->Subscribe_Service_ApplicationResponseProducer(this);
	WebSocketSession_clientApplicationContent subscriptionToken = session->getSubscriptionToken(this);
	printf("%s\n", subscriptionToken.content);
	WebSocketApplicationResponseLogger* logger = new WebSocketApplicationResponseLogger(_application_response_producers[session]);
	return nullptr;
}

void WebSocket_Log_Service::onSessionClosed(WebSocket_Session * session)
{
	if (_application_response_producers.find(session) != _application_response_producers.end())
	{
		_application_response_producers[session]->RemoveAllDestinations();
		_application_response_producers.erase(session);
	}

}

void WebSocket_Log_Service::stop_service()
{
	for (auto iterator : _application_response_producers)
	{
		iterator.second->RemoveAllDestinations();
	}
}


void WebSocketApplicationResponseLogger::OnReceive(WebSocketSession_content content)
{
	if (content.length > 0)
	{
		content.content[content.length] = '\0';
	}
	printf("%s \n", content.content);
}

void WebSocketApplicationResponseLogger::OnReceive(WebSocketSession_clientApplicationContent content)
{
	if (content.length > 0)
	{
		content.content[content.length] = '\0';
	}
	printf("%s \n", content.content);
}
