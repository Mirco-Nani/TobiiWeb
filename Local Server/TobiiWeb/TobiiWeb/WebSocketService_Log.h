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