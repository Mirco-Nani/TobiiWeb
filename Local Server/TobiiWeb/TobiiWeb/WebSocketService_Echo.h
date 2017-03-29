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
