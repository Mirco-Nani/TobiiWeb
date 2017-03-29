#include "stdafx.h"
#include "WebSocket_Services.h"
#include "Utils.h"
#include "Windows_sensing.h"
#include <iostream>
#include <stdio.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

using namespace std;

WebSocket_GazeTracking_Service::WebSocket_GazeTracking_Service(int log_level)
{
	_logLevel = log_level;
	_gazeTracker = TobiiEyeX_GlobalGazeTracker::Instance();
	_gazeTracker_logSource = new ET_Producer<ET_Log>();

	_coordinatesSource = new ET_Producer<ET_GazeCoordinates_Content>();
	_windowPerceiver = new WindowPerceiver(_coordinatesSource);
	_windowInfoSource = new ET_Producer<ET_WindowInfo_Content>();

	_websocket_producer = new ET_Producer<WebSocketSession_content>();
	_translator = new WindowInfo_to_WebSocketSession_contentTranslator(_windowInfoSource, _websocket_producer);


	_windowPerceiver->Init(_windowInfoSource);
	_gazeTracker->Init(_coordinatesSource, _gazeTracker_logSource);

	if (_logLevel >= 1)
	{
		_eyeX_logDestination = new ET_Logger(_gazeTracker_logSource);
	}
	if (_logLevel >= 2)
	{
		_coordinatesLogger = new ET_Logger(_coordinatesSource);
		_windowInfoDestination = new ET_Logger(_windowInfoSource);
	}
}
ET_Producer<WebSocketSession_content>* WebSocket_GazeTracking_Service::onNewSubscription(WebSocket_Session* session)
{
	return _websocket_producer;
}

void WebSocket_GazeTracking_Service::start_service()
{
	_gazeTracker->start(TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED);
}

void WebSocket_GazeTracking_Service::stop_service()
{
	_gazeTracker->stop();
}

void WebSocket_Generic_Service::start()
{
	if (!_running)
	{
		start_service();
		_running = true;
	}
}

void WebSocket_Generic_Service::stop()
{
	if (_running)
	{
		stop_service();
		_running = false;
	}
}

void WebSocket_Session::Emit_Frame(string service, char * frame, int length)
{
	if (length > 0)
	{
		frame[length] = '\0';
	}
	if ( !(_application_response_producers.find(service) == _application_response_producers.end()) )
	{//key exists in map
		_application_response_producers[service]->Emit(WebSocketSession_clientApplicationContent(service, frame, length));
	}
	
}
void WebSocket_Session::Emit_Frame(string service, unsigned long hwnd, unsigned long ancestor_hwnd, char * frame, int length)
{
	if (length > 0)
	{
		frame[length] = '\0';
	}
	if (!(_application_response_producers.find(service) == _application_response_producers.end()))
	{//key exists in map
		_application_response_producers[service]->Emit(WebSocketSession_clientApplicationContent(service, frame, length, hwnd, ancestor_hwnd));
	}
}

void WebSocket_Session::subscribe(WebSocket_Generic_Service * service)
{
	attach_consumer_to_service(service, new WebSocket_ServiceFrame_Consumer());
}

void WebSocket_Session::subscribe(WebSocket_Generic_Service * service, WebSocketSession_clientApplicationContent subscription_token)
{
	//_subscription_tokens[service->getName()] = subscription_token; 
	//the map's [] operator requires the left part of "=" to have a default constructor, so we use instead:
	_subscription_tokens.insert_or_assign(service->getName(), subscription_token);
	subscribe(service);
}

WebSocketSession_clientApplicationContent WebSocket_Session::getSubscriptionToken(WebSocket_Generic_Service * service)
{

	if (_subscription_tokens.find(service->getName()) == _subscription_tokens.end())
	{
		return invalid_SubscriptionToken();
	}
	else
	{
		return _subscription_tokens.find(service->getName())->second;
	}
}

ET_Producer<WebSocketSession_clientApplicationContent>* WebSocket_Session::Subscribe_Service_ApplicationResponseProducer(WebSocket_Generic_Service * service)
{
	ET_Producer<WebSocketSession_clientApplicationContent>* producer = new ET_Producer<WebSocketSession_clientApplicationContent>();
	_application_response_producers[service->getName()] = producer;
	return producer;
}


void WebSocket_Session::close()
{
	for (auto iterator : _consumers)
	{
		iterator.second->Detach();
	}
	for (auto iterator : _services)
	{
		iterator.second->onSessionClosed(this);
	}
}

void WebSocket_Session::attach_consumer_to_service(WebSocket_Generic_Service * service, WebSocket_ServiceFrame_Consumer * consumer)
{
	_services[service->getName()] = service;
	_consumers[service->getName()] = consumer;
	//ET_Producer<WebSocketSession_content>* producer = service->get_WebSocketSessionContent_Producer();
	ET_Producer<WebSocketSession_content>* producer = service->onNewSubscription(this);
	if (producer != nullptr)
	{
		_consumers[service->getName()]->Attach(producer);
	}

	
	//service->onNewSubscription(this);
}


void WindowInfo_to_WebSocketSession_contentTranslator::setProducer(ET_Producer<WebSocketSession_content>* producer)
{
	_producer = producer;
}

ET_Producer<WebSocketSession_content>* WindowInfo_to_WebSocketSession_contentTranslator::getProducer()
{
	return _producer;
}

void WindowInfo_to_WebSocketSession_contentTranslator::OnReceive(ET_WindowInfo_Content content)
{
	if (_producer != nullptr)
	{
		char json[FRAME_LENGHT];
		double x = content.global_gaze_x - content.window_x;
		double y = content.global_gaze_y - content.window_y;
		double timestamp = content.global_gaze_timestamp;
		int len = sprintf_s(json, "{\"type\":\"gaze_coordinates\",\"x\":%f,\"y\":%f,\"timestamp\":%f,\"global_x\":%f,\"global_y\":%f,\"viewport_x\":%f,\"viewport_y\":%f}",
			x, y, timestamp, content.global_gaze_x, content.global_gaze_y, content.window_x, content.window_y);
		_producer->Emit(WebSocketSession_content(json, len, content.window_hwnd, content.ancestorWindow_hwnd));
	}
}

void WebSocket_ServiceFrame_Consumer::OnReceive(WebSocketSession_content content)
{
	if (content.window_needed)
	{
		if (content.hwnd == _hwnd && content.ancestor_hwnd == _ancestor_hwnd)
		{
			sendContent(content);
		}
	}
	else
	{
		sendContent(content);
	}
}

void WebSocket_ServiceFrame_Consumer::set_hwnds(unsigned long hwnd, unsigned long ancestor_hwnd)
{
	_hwnd = hwnd;
	_ancestor_hwnd = ancestor_hwnd;
}

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

WebSocketSession_clientApplicationContent invalid_SubscriptionToken()
{
	return WebSocketSession_clientApplicationContent("unknown", nullptr, -1);
}

bool is_SubscriptionToken_Valid(WebSocketSession_clientApplicationContent token)
{
	if (token.service_name == "unknown" || token.content == nullptr || token.length < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}


ET_Producer<WebSocketSession_content>* WebSocket_Service::onNewSubscription(WebSocket_Session * session)
{
	WebSocketSession_clientApplicationContent subscriptionToken = session->getSubscriptionToken(this);
	if (!is_SubscriptionToken_Valid(subscriptionToken)) return nullptr;
	string session_id = session->get_sessionID();

	// input
	ET_Producer<WebSocketSession_clientApplicationContent>* inputSource = session->Subscribe_Service_ApplicationResponseProducer(this);
	ET_Producer<WebSocketSession_Message>* inputDestination = getInputProducer(session_id);//new ET_Producer<WebSocketSession_Message>();
	WebSocketMessageInputForwarder* inputForwarder = new WebSocketMessageInputForwarder(inputSource, inputDestination, session_id);

	// output
	ET_Producer<WebSocketSession_content>* outputDestination = new ET_Producer<WebSocketSession_content>();
	ET_Producer<WebSocketSession_Message>* outputSource = getOutputProducer();
	//cout << this->getName() << endl;
	WebSocketMessageOutputForwarder* outputForwarder = new WebSocketMessageOutputForwarder(outputSource, outputDestination, session_id, this->getName());

	session_generic_resource sgr;
	sgr.producers->push_back(inputSource);
	sgr.producers->push_back(inputDestination);
	sgr.consumers->push_back(inputForwarder);
	sgr.producers->push_back(outputDestination);
	sgr.producers->push_back(outputSource);
	sgr.consumers->push_back(outputForwarder);

	WebSocketSession_clientApplicationContent token = session->getSubscriptionToken(this);
	WebSocketSession_Message* messageToken;
	if(token.window_needed)
	{
		messageToken = new WebSocketSession_Message(session->get_sessionID(), token.content, token.length, token.hwnd, token.ancestor_hwnd);
	}
	else
	{
		messageToken = new WebSocketSession_Message(session->get_sessionID(), token.content, token.length);
	}

	onNewSession(inputDestination, outputSource, sgr.producers, sgr.consumers, messageToken);

	_session_generic_resources.insert_or_assign(session, sgr);

	return outputDestination;
}

void WebSocket_Service::onSessionClosed(WebSocket_Session * session)
{
	map<WebSocket_Session*, session_generic_resource>::iterator it = _session_generic_resources.find(session);
	if (it != _session_generic_resources.end()) 
	{
		std::vector<ET_Consumer*>* consumers = it->second.consumers;
		while (!consumers->empty())
		{
			ET_Consumer* producer = consumers->back();
			producer->Detach();
			consumers->pop_back();
		}

		std::vector<ET_Generic_Producer*>* producers = it->second.producers;
		while (!producers->empty())
		{
			ET_Generic_Producer* producer = producers->back();
			producer->RemoveAllDestinations();
			producers->pop_back();
		}
		_session_generic_resources.erase(session);
	}
	

	map<string, ET_Producer<WebSocketSession_Message>*>::iterator it2 = _session_input_producers.find(session->get_sessionID());
	if (it2 != _session_input_producers.end())
	{
		it2->second->RemoveAllDestinations();
		_session_input_producers.erase(session->get_sessionID());
	}


}

ET_Producer<WebSocketSession_Message>* WebSocket_Service::getOutputProducer()
{
	if (_outputProducer == nullptr)
	{
		_outputProducer = new ET_Producer<WebSocketSession_Message>();
	}
	return _outputProducer;
}

ET_Producer<WebSocketSession_Message>* WebSocket_Service::getInputProducer(string session_id)
{
	map<string, ET_Producer<WebSocketSession_Message>*>::iterator it = _session_input_producers.find(session_id);
	if (it != _session_input_producers.end())
	{
		return it->second;
	}
	else 
	{
		//throw "Requested uninitialized input producer for session ID: "+session_id;
		ET_Producer<WebSocketSession_Message>* inputProducer = new ET_Producer<WebSocketSession_Message>();
		_session_input_producers.insert_or_assign(session_id, inputProducer);
		return inputProducer;
	}
}

void WebSocket_Service::WebSocketMessageOutputForwarder::OnReceive(WebSocketSession_Message message)
{
	if(_session_id == message.session_id)
	{
		string content = message.getContent();//chars_to_string(message.content, message.length);
		string json = "{ \"content\": "+ content +", \"service\":\""+ _service_name +"\", \"type\": \"service_message\"}";
		message.setContent(json);
		_destination->Emit(message);
	}
}

void WebSocket_Service::WebSocketMessageInputForwarder::OnReceive(WebSocketSession_clientApplicationContent content)
{
	if (content.window_needed)
	{
		_destination->Emit(WebSocketSession_Message(_session_id, content.content, content.length, content.hwnd, content.ancestor_hwnd));
	}
	else
	{
		_destination->Emit(WebSocketSession_Message(_session_id, content.content, content.length));
	}
	
}

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

	message.setContent("\""+ _prepend + content +"\"");
	_echoDestination->Emit(message);
}

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
