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
		_coordinatesLogger = new ET_Consumer(_coordinatesSource);
		_windowInfoDestination = new ET_Consumer(_windowInfoSource);
	}
}
ET_Producer<WebSocketSession_content>* WebSocket_GazeTracking_Service::onNewSubscription(WebSocket_Session* session)
{
	return _websocket_producer;
}
/*
ET_Producer<WebSocketSession_content>* WebSocket_GazeTracking_Service::get_WebSocketSessionContent_Producer()
{
	return _websocket_producer;
}
*/
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

ScreenshotFile_Service::ScreenshotFile_Service()
{
}

ET_Producer<WebSocketSession_content>* ScreenshotFile_Service::onNewSubscription(WebSocket_Session * session)
{
	session_stuff sessionStuff;

	WebSocketSession_clientApplicationContent subscriptionToken = session->getSubscriptionToken(this);
	if (!is_SubscriptionToken_Valid(subscriptionToken)) return nullptr;

	if (!subscriptionToken.window_needed)
	{
		cout << "subscription request without window: request refused" << endl;
		return nullptr;
	}

	sessionStuff.hwnd = subscriptionToken.hwnd;
	
	Document d;
	d.Parse(subscriptionToken.content);
	if (d.HasMember("activation"))
	{
		if (d["activation"].IsString())
		{
			sessionStuff.activation = d["activation"].GetString();
		}
	}
	if (d.HasMember("activation_period"))
	{
		if (d["activation_period"].IsInt())
		{
			sessionStuff.activation_period = d["activation_period"].GetInt();
		}
	}
	/* //EVENT-DRIVEN ACTIVATION TIME NOT IMPLEMENTED FOR NOW
	if (d.HasMember("activation_time"))
	{
		if (d["activation_time"].IsString())
		{
			sessionStuff.activation_time = d["activation_time"].GetString();
		}
	}
	*/
	string subscriptionToken_content(subscriptionToken.content);
	string sessionID = session->get_sessionID();
	
	ET_Producer<ScreenshotMetadata_content>* screenshotMetadata_source = new ET_Producer<ScreenshotMetadata_content>();

	if (sessionID != "unknown")
	{
		sessionStuff.sessionID = sessionID;
		ScreenshotMetadata_translator* screenshotMetadata_translator = new ScreenshotMetadata_translator(screenshotMetadata_source);
		screenshotMetadata_translator->setSessionID(sessionID);
		screenshotMetadata_translator->setSubscriptionToken(subscriptionToken_content);
		Datasetore_SessionPersister_Consumer* sessionPersisterConsumer = Datasetore_Session_Persister::Instance()->getPersisterConsumer(sessionID);
		ET_Producer<SessionPersistence_Content>* sessionPersistanceProducer = new ET_Producer<SessionPersistence_Content>();
		sessionPersisterConsumer->Attach(sessionPersistanceProducer);
		screenshotMetadata_translator->set_ScreenshotPersistence_producer(sessionPersistanceProducer);

		sessionStuff.screenshot_metadata_consumer = screenshotMetadata_translator;
		sessionStuff.session_persistance_producer = sessionPersistanceProducer;
		sessionStuff.datastore_persister_consumer = sessionPersisterConsumer;
	}
	else
	{
		ScreenshotMetadata_Logger* metadata_logger = new ScreenshotMetadata_Logger(screenshotMetadata_source);
		sessionStuff.screenshot_metadata_consumer = metadata_logger;
	}

	if (sessionStuff.activation == "periodic")
	{
		Periodic_WindowScreenshotRequestor* periodic_screenshot_taker = new Periodic_WindowScreenshotRequestor(sessionStuff.hwnd, sessionID);
		ET_Producer<ScreenshotRequest_content>* screenshot_requestor = periodic_screenshot_taker->getProducer();

		ScreenshotTaker* screensTaker = new ScreenshotTaker(screenshot_requestor, screenshotMetadata_source);

		sessionStuff.screenshot_metadata_producer = screenshotMetadata_source;
		sessionStuff.periodic_screenshot_requestor = periodic_screenshot_taker;
		sessionStuff.screenshot_requestor = screenshot_requestor;
		sessionStuff.screenshot_taker = screensTaker;

		if (sessionStuff.activation_time == "now")
		{
			if (sessionStuff.activation_period > 0)
			{
				periodic_screenshot_taker->start(sessionStuff.activation_period);
			}
			else
			{
				periodic_screenshot_taker->start();
			}
			
		}
	}
	if (sessionStuff.activation == "event")
	{
		ET_Producer<WebSocketSession_clientApplicationContent>* client_applicationResponse_producer = session->Subscribe_Service_ApplicationResponseProducer(this);
		ScreenshotEvent_translator* event_transaltor = new ScreenshotEvent_translator(client_applicationResponse_producer, sessionID);


		ET_Producer<ScreenshotRequest_content>* screenshot_requestor = new ET_Producer<ScreenshotRequest_content>();
		event_transaltor->set_ScreenshotRequest_producer(screenshot_requestor);
		ScreenshotTaker* screensTaker = new ScreenshotTaker(screenshot_requestor, screenshotMetadata_source);

		sessionStuff.screenshot_metadata_producer = screenshotMetadata_source;
		sessionStuff.screenshot_requestor = screenshot_requestor;
		sessionStuff.screenshot_taker = screensTaker;

		sessionStuff.application_response_producer = client_applicationResponse_producer;
		sessionStuff.screenshot_event_translator = event_transaltor;
	}

	_sessions_stuff.insert_or_assign(session, sessionStuff);

	return nullptr;
}

void ScreenshotFile_Service::onSessionClosed(WebSocket_Session * session)
{
	session_stuff sessionStuff = _sessions_stuff[session];
	if (sessionStuff.periodic_screenshot_requestor != nullptr)
	{
		if (sessionStuff.periodic_screenshot_requestor->is_running())
		{
			sessionStuff.periodic_screenshot_requestor->stop();
		}
	}
	if (sessionStuff.screenshot_metadata_producer != nullptr)
	{
		sessionStuff.screenshot_metadata_producer->RemoveAllDestinations();
	}
	if (sessionStuff.screenshot_metadata_consumer != nullptr)
	{
		sessionStuff.screenshot_metadata_consumer->Detach();
	}
	if (sessionStuff.screenshot_requestor != nullptr)
	{
		sessionStuff.screenshot_requestor->RemoveAllDestinations();
	}
	if (sessionStuff.screenshot_taker != nullptr)
	{
		sessionStuff.screenshot_taker->Detach();
	}
	if (sessionStuff.application_response_producer != nullptr)
	{
		sessionStuff.application_response_producer->RemoveAllDestinations();
	}

	if (sessionStuff.datastore_persister_consumer != nullptr)
	{
		sessionStuff.datastore_persister_consumer->flushPersisterBuffer();
		sessionStuff.datastore_persister_consumer->Detach();
	}

	if (sessionStuff.screenshot_event_translator != nullptr)
	{
		sessionStuff.screenshot_event_translator->Detach();
	}
	if (sessionStuff.session_persistance_producer != nullptr)
	{
		sessionStuff.session_persistance_producer->RemoveAllDestinations();
	}
	if (sessionStuff.sessionID != "unknown")
	{
		Datasetore_Session_Persister::Instance()->dismissPersisterConsumer(sessionStuff.sessionID);
	}
	_sessions_stuff.erase(session);
}

void ScreenshotFile_Service::stop_service()
{
	map<WebSocket_Session*, session_stuff> buffer;
	for (auto iterator : _sessions_stuff)
	{
		buffer.insert_or_assign(iterator.first, iterator.second);
	}
	for (auto iterator : buffer)
	{
		onSessionClosed(iterator.first);
	}
	buffer.clear();
	_sessions_stuff.clear();
}

void ScreenshotEvent_translator::set_ScreenshotRequest_producer(ET_Producer<ScreenshotRequest_content>* producer)
{
	_producer = producer;
}

void ScreenshotEvent_translator::OnReceive(WebSocketSession_clientApplicationContent content)
{
	if (_producer == nullptr)
	{
		cout << "ScreenshotEvent_translator: producer is not set!!!" << endl;
		return;
	}
	if (!content.window_needed)
	{
		cout << "ScreenshotEvent_translator: window is not set!!!" << endl;
		return;
	}

	Document d;
	d.Parse(content.content);
	if (d.HasMember("event"))
	{
		string evt = d["event"].GetString();
		
		if (evt.compare("take_screenshot") == 0)
		{
			_producer->Emit(ScreenshotRequest_content(content.hwnd, "../media/" + _sessionID + "_" + to_string(_screensCount) + ".jpg"));
			_screensCount++;
		}
	}
}

void ScreenshotMetadata_translator::setSessionID(string sessionID)
{
	_sessionID = sessionID;
}

void ScreenshotMetadata_translator::setSubscriptionToken(string subscriptionToken)
{
	_subscriptionToken = subscriptionToken;
}

void ScreenshotMetadata_translator::set_ScreenshotPersistence_producer(ET_Producer<SessionPersistence_Content>* producer)
{
	_producer = producer;
}

void ScreenshotMetadata_translator::OnReceive(ScreenshotMetadata_content content)
{
	string json = "{\"session_id\":\"" + _sessionID + "\",\"content_type\":\"screenshot\",\"content\":{\"path\":\"" + content.location_path + "\",\"timestamp\":\"" + to_string(content.timestamp) + "\"}";
	if (_subscriptionToken != "unknown")
	{
		json += ",\"subscription_token\":" + _subscriptionToken;
	}
	json += "}";
	_producer->Emit(SessionPersistence_Content(_sessionID, json));
}

ET_Producer<WebSocketSession_content>* WebSocket_ClientData_Service::onNewSubscription(WebSocket_Session * session)
{
	string sessionID = session->get_sessionID();

	if (sessionID != "unknown")
	{
		WebSocketSession_clientApplicationContent subscriptionToken = session->getSubscriptionToken(this);
		ET_Producer<WebSocketSession_clientApplicationContent>* client_application_content_producer = session->Subscribe_Service_ApplicationResponseProducer(this);

		ClientApplicationContent_translator* application_content_translator = new ClientApplicationContent_translator(client_application_content_producer);
		
		ET_Producer<SessionPersistence_Content>* sessionPersistance_producer = new ET_Producer<SessionPersistence_Content>();

		application_content_translator->setSessionID(sessionID);
		application_content_translator->setSubscriptionToken(subscriptionToken);
		application_content_translator->set_SessionPersistence_producer(sessionPersistance_producer);

		Datasetore_SessionPersister_Consumer* datastore_persistence_consumer = Datasetore_Session_Persister::Instance()->getPersisterConsumer(sessionID);
		datastore_persistence_consumer->Attach(sessionPersistance_producer);

		session_stuff sessionStuff;
		sessionStuff.sessionID = sessionID;
		sessionStuff.client_application_content_producer = client_application_content_producer;
		sessionStuff.application_content_translator = application_content_translator;
		sessionStuff.session_persistance_producer = sessionPersistance_producer;
		sessionStuff.datastore_persistence_consumer = datastore_persistence_consumer;

		_sessions_stuff[session] = sessionStuff;
	}

	return nullptr;
}

void WebSocket_ClientData_Service::onSessionClosed(WebSocket_Session * session)
{
	if (_sessions_stuff.find(session) != _sessions_stuff.end())
	{
		if (_sessions_stuff[session].sessionID != "unknown")
		{
			if (_sessions_stuff[session].datastore_persistence_consumer != nullptr)
			{
				_sessions_stuff[session].datastore_persistence_consumer->flushPersisterBuffer();
				_sessions_stuff[session].datastore_persistence_consumer->Detach();
			}
		}
		if (_sessions_stuff[session].client_application_content_producer != nullptr)
		{
			_sessions_stuff[session].client_application_content_producer->RemoveAllDestinations();
		}
		if (_sessions_stuff[session].application_content_translator != nullptr)
		{
			_sessions_stuff[session].application_content_translator->Detach();
		}
		if (_sessions_stuff[session].session_persistance_producer != nullptr)
		{
			_sessions_stuff[session].session_persistance_producer->RemoveAllDestinations();
		}

		_sessions_stuff.erase(session);
	}
}

void WebSocket_ClientData_Service::stop_service()
{
	map<WebSocket_Session*, session_stuff> buffer;
	for (auto iterator : _sessions_stuff)
	{
		buffer.insert_or_assign(iterator.first, iterator.second);
	}
	for (auto iterator : buffer)
	{
		onSessionClosed(iterator.first);
	}
	buffer.clear();
	_sessions_stuff.clear();
}

void ClientApplicationContent_translator::setSessionID(string sessionID)
{
	_sessionID = sessionID;
}

void ClientApplicationContent_translator::setSubscriptionToken(WebSocketSession_clientApplicationContent subscriptionToken)
{
	string content(subscriptionToken.content);
	_subscriptionTokenContent = content;
}

void ClientApplicationContent_translator::set_SessionPersistence_producer(ET_Producer<SessionPersistence_Content>* producer)
{
	_producer = producer;
}

void ClientApplicationContent_translator::OnReceive(WebSocketSession_clientApplicationContent content)
{
	if (_producer != nullptr)
	{
		string json_clientContent(content.content);
		string json = "{\"session_id\":\"" + _sessionID + "\",\"content_type\":\"gaze_point\",\"content\":" + json_clientContent;
		if (_subscriptionTokenContent != "unknown")
		{
			json += ",\"subscription_token\":" + _subscriptionTokenContent;
		}
		json += "}";
		_producer->Emit(SessionPersistence_Content(_sessionID, json));
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
				string tempImgFilePath = "../media/" + _tempImgFileName + message.session_id + ".jpg";
				screenshotPrinter->TakeScreenshotOfWindow(message.hwnd, tempImgFilePath);
				string base64Img = File_to_Base64(tempImgFilePath);
				remove(tempImgFilePath.c_str());
				WebSocketSession_Message result(message);
				string json = "{\"img\":\"" + base64Img + "\"}"; //"{ \"content\": {\"img\":\"" + base64Img + "\"}, \"service\":\"Screenshot_Service\", \"type\": \"service_message\"}";
				result.setContent(json);
				//result.content = string_to_chars(json);
				//result.length = json.length();
				_destination->Emit(result);
			}
		}
	}
}
