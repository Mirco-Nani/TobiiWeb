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
