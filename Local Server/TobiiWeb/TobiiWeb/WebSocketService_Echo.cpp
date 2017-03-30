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
#include <iostream>
#include "WebSocketService_Echo.h"
#include "rapidjson/document.h"

using namespace rapidjson;
using namespace std;

void Echo_Service::onNewSession(
	ET_Producer<WebSocketSession_Message>* inputProducer,
	ET_Producer<WebSocketSession_Message>* outputProducer,
	std::vector<ET_Generic_Producer*>* producers,
	std::vector<ET_Consumer*>* consumers,
	WebSocketSession_Message* subscriptionToken)
{
	Document d;
	d.Parse(subscriptionToken->content);
	string prepend = "";
	if (d.HasMember("prepend"))
	{
		if (d["prepend"].IsString())
		{
			prepend = d["prepend"].GetString();
		}
	}

	Echo_Forwarder* echoForwarder = new Echo_Forwarder(inputProducer, outputProducer, prepend);
	consumers->push_back(echoForwarder);
}

void Echo_Service::Echo_Forwarder::OnReceive(WebSocketSession_Message message)
{
	//do something with the content...
	cout << "Echo Forwarder received: " << message.content << endl;
	cout << "prepending: " << _prepend << endl;

	Document d;
	d.Parse(message.content);
	string content = "";
	if (d.HasMember("content"))
	{
		if (d["content"].IsString())
		{
			content = d["content"].GetString();
		}
	}

	message.setContent("\"" + _prepend + content + "\"");
	_echoDestination->Emit(message);
}
