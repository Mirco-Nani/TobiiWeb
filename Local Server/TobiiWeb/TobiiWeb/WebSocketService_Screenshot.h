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
