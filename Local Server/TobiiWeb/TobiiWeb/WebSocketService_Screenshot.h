#pragma once

#include "WebSocket_Services.h"

class Screenshot_Service : public WebSocket_Service
{
public:
	virtual string getName() { return "Screenshot_Service"; };
	Screenshot_Service() {};

protected:
	virtual void onNewSession(
		ET_Producer<WebSocketSession_Message>* inputProducer,
		ET_Producer<WebSocketSession_Message>* outputProducer,
		std::vector<ET_Generic_Producer*>* producers,
		std::vector<ET_Consumer*>* consumers,
		WebSocketSession_Message* subscriptionToken);

	class Screenshot_Taker : public ET_Consumer
	{
	public:
		Screenshot_Taker(ET_Generic_Producer* source, ET_Producer<WebSocketSession_Message>* destination) : ET_Consumer(source), _destination(destination) {};
		void OnReceive(WebSocketSession_Message message);
	protected:
		ET_Producer<WebSocketSession_Message>* _destination;
		ScreenshotPrinter* screenshotPrinter = new ScreenshotPrinter();
		string _tempImgFileName = "temp";
	};
};
