#pragma once
#include <map>
#include <vector>

#include "TobiiEyeX_GlobalGazeTracker.h"
#include "Windows_sensing.h"
#include "Screenshot.h"
#include "Persistence.h"

#define FRAME_LENGHT 1024

class WebSocket_Generic_Service;


WebSocketSession_clientApplicationContent invalid_SubscriptionToken();

bool is_SubscriptionToken_Valid(WebSocketSession_clientApplicationContent token);

class WebSocket_ServiceFrame_Consumer : public ET_Consumer_Of<WebSocketSession_content>
{
public:
	virtual void OnReceive(WebSocketSession_content content);
	virtual void set_hwnds(unsigned long hwnd, unsigned long ancestor_hwnd);
protected:
	virtual void sendContent(WebSocketSession_content content) {};
	unsigned long _hwnd;
	unsigned long _ancestor_hwnd;
};

//ToDo:
// modificare POCO_WebSocket_Session e POCO_WebSocketServer per far emettere a _application_response_producer dei WebSocketSession_content quando il client manda dei frame durante la sessione!
class WebSocket_Session {
public:
	WebSocket_Session() {};

	virtual void set_hwnd(unsigned long hwnd)
	{
		_hwnd = hwnd;
	}
	virtual void set_ancestor_hwnd(unsigned long hwnd)
	{
		_ancestor_hwnd = hwnd;
	}
	virtual void set_sessionID(string sessionID)
	{
		_id = sessionID;
	}
	virtual string get_sessionID() { return _id; };

	void Emit_Frame(string service, char* frame, int length = -1); //emits a frame TO A SERVICE
	void Emit_Frame(string service, unsigned long hwnd, unsigned long ancestor_hwnd, char* frame, int length = -1); //emits a frame TO A SERVICE
	virtual void subscribe(WebSocket_Generic_Service* service);
	virtual void subscribe(WebSocket_Generic_Service* service, WebSocketSession_clientApplicationContent subscription_token);

	virtual WebSocketSession_clientApplicationContent getSubscriptionToken(WebSocket_Generic_Service* service);

	virtual ET_Producer<WebSocketSession_clientApplicationContent>* Subscribe_Service_ApplicationResponseProducer(WebSocket_Generic_Service* service);

	virtual void close();

protected:
	virtual void attach_consumer_to_service(WebSocket_Generic_Service* service, WebSocket_ServiceFrame_Consumer* consumer);

	unsigned long _hwnd;
	unsigned long _ancestor_hwnd;
	string _id = "unknown";

	map<string, WebSocket_Generic_Service*> _services;
	map<string, WebSocket_ServiceFrame_Consumer*> _consumers;

	map<string, ET_Producer<WebSocketSession_clientApplicationContent>*> _application_response_producers;
	map<string, WebSocketSession_clientApplicationContent> _subscription_tokens;


};

class WebSocket_Generic_Service
{
public:
	virtual string getName() { return "WebSocket_Generic_Service"; };
	virtual void start();
	virtual void stop();
	//virtual void send() {};
	//virtual void onFrameReceived(WebSocketSession_content frame) {};
	//virtual void onNewSubscription(WebSocket_Session* session) {};
	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session) { return nullptr; };
	virtual void onSessionClosed(WebSocket_Session* session){}
	//virtual ET_Producer<WebSocketSession_content>* get_WebSocketSessionContent_Producer() { return nullptr; };

protected:
	bool _running = false;
	map<WebSocket_Session*, ET_Producer<WebSocketSession_clientApplicationContent>*> _application_response_producers;

	virtual void start_service() {};
	virtual void stop_service() {};
};


class WebSocket_Service : public WebSocket_Generic_Service 
{
public:
	virtual string getName() { return "WebSocket_Service"; };
	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	virtual void onSessionClosed(WebSocket_Session* session);
	//virtual void onSessionClosed(WebSocket_Session* session);

protected:
	virtual void onNewSession(
		ET_Producer<WebSocketSession_Message>* inputProducer, 
		ET_Producer<WebSocketSession_Message>* outputProducer, 
		std::vector<ET_Generic_Producer*>* producers, 
		std::vector<ET_Consumer*>* consumers){}


	virtual ET_Producer<WebSocketSession_Message>* getOutputProducer();
	virtual ET_Producer<WebSocketSession_Message>* getInputProducer(string session_id);
	ET_Producer<WebSocketSession_Message>* _outputProducer = nullptr;
	//ET_Producer<WebSocketSession_Message>* _inputProducer = nullptr;
	map<string, ET_Producer<WebSocketSession_Message>*> _session_input_producers;

	class WebSocketMessageOutputForwarder : public ET_Consumer
	{
	public:
		WebSocketMessageOutputForwarder(ET_Producer<WebSocketSession_Message>* source, ET_Producer<WebSocketSession_content>* destination, string session_id, string service_name)
			: ET_Consumer(source), _destination(destination), _session_id(session_id), _service_name(service_name){};
		virtual void OnReceive(WebSocketSession_Message content);
	private:
		string _session_id;
		string _service_name;
		ET_Producer<WebSocketSession_content>* _destination;
	};

	class WebSocketMessageInputForwarder : public ET_Consumer
	{
	public:
		WebSocketMessageInputForwarder(ET_Producer<WebSocketSession_clientApplicationContent>* source, ET_Producer<WebSocketSession_Message>* destination, string session_id)
			: ET_Consumer(source), _destination(destination), _session_id(session_id) {};
		virtual void OnReceive(WebSocketSession_clientApplicationContent content);
	private:
		string _session_id;
		ET_Producer<WebSocketSession_Message>* _destination;
	};

	
	//WebSocketMessageOutputForwarder* _webSocketMessageOutputForwarder = nullptr;

	struct session_generic_resource
	{
		std::vector<ET_Generic_Producer*>* producers = new std::vector<ET_Generic_Producer*>();
		std::vector<ET_Consumer*>* consumers = new std::vector<ET_Consumer*>();
	};
	map<WebSocket_Session*, session_generic_resource> _session_generic_resources;

};

class WindowInfo_to_WebSocketSession_contentTranslator : public ET_Consumer_Of<ET_WindowInfo_Content>
{
public:
	WindowInfo_to_WebSocketSession_contentTranslator(ET_Producer<ET_WindowInfo_Content>* source, ET_Producer<WebSocketSession_content>* producer) 
		: _producer(producer), ET_Consumer_Of<ET_WindowInfo_Content>(source) {};
	virtual void OnReceive(ET_WindowInfo_Content content);

	virtual void setProducer(ET_Producer<WebSocketSession_content>* producer);
	virtual ET_Producer<WebSocketSession_content>* getProducer();
private:
	ET_Producer<WebSocketSession_content>* _producer = nullptr;
};


class WebSocket_GazeTracking_Service : public WebSocket_Generic_Service
{
public:
	virtual string getName() { return "GazeTracking_Service"; };
	WebSocket_GazeTracking_Service(int log_level = 0);
	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	//virtual ET_Producer<WebSocketSession_content>* get_WebSocketSessionContent_Producer();

protected:
	virtual void start_service();
	virtual void stop_service();

	unsigned int _logLevel;
	TobiiEyeX_GlobalGazeTracker* _gazeTracker;
	ET_Producer<ET_GazeCoordinates_Content>* _coordinatesSource;
	WindowPerceiver* _windowPerceiver;
	ET_Producer<ET_WindowInfo_Content>* _windowInfoSource;
	WindowInfo_to_WebSocketSession_contentTranslator* _translator;
	ET_Producer<WebSocketSession_content>* _websocket_producer;

	//log level 1
	ET_Producer<ET_Log>* _gazeTracker_logSource;
	ET_Logger* _eyeX_logDestination;

	// log level 2
	ET_Consumer* _coordinatesLogger;
	ET_Consumer* _windowInfoDestination;
};


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

class ClientApplicationContent_translator : public ET_Consumer
{
public:
	ClientApplicationContent_translator(ET_Generic_Producer* producer) : ET_Consumer(producer) {};
	void setSessionID(string sessionID);
	void setSubscriptionToken(WebSocketSession_clientApplicationContent subscriptionToken);
	void set_SessionPersistence_producer(ET_Producer<SessionPersistence_Content>* producer);
	void OnReceive(WebSocketSession_clientApplicationContent content);
private:
	ET_Producer<SessionPersistence_Content>* _producer = nullptr;
	string _sessionID = "unknown";
	string _subscriptionTokenContent = "unknown";
};


class WebSocket_ClientData_Service : public WebSocket_Generic_Service
{
public:
	virtual string getName() { return "ClientData_Service"; };
	WebSocket_ClientData_Service() {};

	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	virtual void onSessionClosed(WebSocket_Session* session);
protected:
	virtual void stop_service();
	
	struct session_stuff
	{
		string sessionID = "unknown";

		ET_Producer<WebSocketSession_clientApplicationContent>* client_application_content_producer = nullptr;
		ClientApplicationContent_translator* application_content_translator = nullptr;
		ET_Producer<SessionPersistence_Content>* session_persistance_producer = nullptr;
		Datasetore_SessionPersister_Consumer* datastore_persistence_consumer = nullptr;
	};
	map<WebSocket_Session*, session_stuff> _sessions_stuff;

};

class ScreenshotEvent_translator: public ET_Consumer
{
public:
	ScreenshotEvent_translator(ET_Generic_Producer* producer, string sessionID) : ET_Consumer(producer), _sessionID(sessionID) {};
	void set_ScreenshotRequest_producer(ET_Producer<ScreenshotRequest_content>* producer);
	void OnReceive(WebSocketSession_clientApplicationContent content);
private:
	ET_Producer<ScreenshotRequest_content>* _producer = nullptr;
	string _sessionID = "unknown";
	int _screensCount = 0;
};

class ScreenshotMetadata_translator : public ET_Consumer
{
public:
	ScreenshotMetadata_translator(ET_Generic_Producer* producer) : ET_Consumer(producer) {};
	void setSessionID(string sessionID);
	void setSubscriptionToken(string subscriptionToken);
	void set_ScreenshotPersistence_producer(ET_Producer<SessionPersistence_Content>* producer);
	void OnReceive(ScreenshotMetadata_content content);
private:
	ET_Producer<SessionPersistence_Content>* _producer = nullptr;
	string _sessionID = "unknown";
	string _subscriptionToken = "unknown";
};

class ScreenshotFile_Service : public WebSocket_Generic_Service
{
public:
	virtual string getName() { return "ScreenshotFile_Service"; };
	ScreenshotFile_Service();

	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session);
	virtual void onSessionClosed(WebSocket_Session* session);
protected:
	virtual void stop_service();

	struct session_stuff
	{
		string activation = "";
		int activation_period = -1;
		string activation_time = "now";
		unsigned int hwnd;

		ET_Producer<ScreenshotMetadata_content>* screenshot_metadata_producer = nullptr;
		ET_Consumer* screenshot_metadata_consumer = nullptr;
		Periodic_WindowScreenshotRequestor* periodic_screenshot_requestor = nullptr;
		ET_Producer<ScreenshotRequest_content>* screenshot_requestor = nullptr;
		ScreenshotTaker* screenshot_taker = nullptr;

		ET_Producer<WebSocketSession_clientApplicationContent>* application_response_producer = nullptr;
		ScreenshotEvent_translator* screenshot_event_translator = nullptr;

		ET_Producer<SessionPersistence_Content>* session_persistance_producer = nullptr;
		Datasetore_SessionPersister_Consumer* datastore_persister_consumer = nullptr;
		string sessionID = "unknown";
	};
	map<WebSocket_Session*, session_stuff> _sessions_stuff;
};

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
		std::vector<ET_Consumer*>* consumers);

	class Echo_Forwarder : public ET_Consumer
	{
	public:
		Echo_Forwarder(ET_Generic_Producer* echoSource, ET_Producer<WebSocketSession_Message>* echoDestination) : ET_Consumer(echoSource), _echoDestination(echoDestination) {};
		void OnReceive(WebSocketSession_Message message);
	protected:
		ET_Producer<WebSocketSession_Message>* _echoDestination;
	};
};

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
		std::vector<ET_Consumer*>* consumers);

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