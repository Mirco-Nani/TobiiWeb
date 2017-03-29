#include "stdafx.h"
#include "WebSocketService_Screenshot.h"
#include "rapidjson/document.h"
#include "Utils.h"

using namespace rapidjson;

void Screenshot_Service::onNewSession(
	ET_Producer<WebSocketSession_Message>* inputProducer,
	ET_Producer<WebSocketSession_Message>* outputProducer,
	std::vector<ET_Generic_Producer*>* producers,
	std::vector<ET_Consumer*>* consumers,
	WebSocketSession_Message* subscriptionToken)
{
	Screenshot_Taker* screenshotTaker = new Screenshot_Taker(inputProducer, outputProducer);
	consumers->push_back(screenshotTaker);
}

void Screenshot_Service::Screenshot_Taker::OnReceive(WebSocketSession_Message message)
{
	Document d;
	d.Parse(message.content);
	if (d.HasMember("command"))
	{
		if (d["command"].IsString())
		{
			string command = d["command"].GetString();
			if (command == "take_screenshot")
			{
				string tempImgFilePath = "./" + _tempImgFileName + message.session_id + ".jpg";
				screenshotPrinter->TakeScreenshotOfWindow(message.hwnd, tempImgFilePath);
				string base64Img = File_to_Base64(tempImgFilePath);
				remove(tempImgFilePath.c_str());
				WebSocketSession_Message result(message);
				string json = "{\"img\":\"" + base64Img + "\"}";
				result.setContent(json);
				_destination->Emit(result);
			}
		}
	}
}
