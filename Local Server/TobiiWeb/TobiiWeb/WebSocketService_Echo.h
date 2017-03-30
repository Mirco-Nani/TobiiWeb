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

class Echo_Service : public WebSocket_Service
{
public:
	virtual string getName() { return "Echo_Service"; };
	Echo_Service() {};
protected:
	virtual void onNewSession(
		ET_Producer<WebSocketSession_Message>* inputProducer,
		ET_Producer<WebSocketSession_Message>* outputProducer,
		std::vector<ET_Generic_Producer*>* producers,
		std::vector<ET_Consumer*>* consumers,
		WebSocketSession_Message* subscriptionToken);

	class Echo_Forwarder : public ET_Consumer
	{
	public:
		Echo_Forwarder(ET_Generic_Producer* echoSource, ET_Producer<WebSocketSession_Message>* echoDestination, string prepend)
			: ET_Consumer(echoSource), _echoDestination(echoDestination), _prepend(prepend) {};
		void OnReceive(WebSocketSession_Message message);
	protected:
		ET_Producer<WebSocketSession_Message>* _echoDestination;
		string _prepend = "";
	};
};
