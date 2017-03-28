#pragma once
#include <map>
#include <vector>

#include "TobiiEyeX_GlobalGazeTracker.h"
#include "Windows_sensing.h"

#define FRAME_LENGHT 1024

class WebSocket_Generic_Service;


WebSocketSession_clientApplicationContent invalid_SubscriptionToken();

bool is_SubscriptionToken_Valid(WebSocketSession_clientApplicationContent token);

class WebSocket_ServiceFrame_Consumer : public ET_Consumer
{
public:
	virtual void OnReceive(WebSocketSession_content content);
	virtual void set_hwnds(unsigned long hwnd, unsigned long ancestor_hwnd);
protected:
	virtual void sendContent(WebSocketSession_content content) {};
	unsigned long _hwnd;
	unsigned long _ancestor_hwnd;
};


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
	virtual ET_Producer<WebSocketSession_content>* onNewSubscription(WebSocket_Session* session) { return nullptr; };
	virtual void onSessionClosed(WebSocket_Session* session){}

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

protected:
	virtual void onNewSession(
		ET_Producer<WebSocketSession_Message>* inputProducer, 
		ET_Producer<WebSocketSession_Message>* outputProducer, 
		std::vector<ET_Generic_Producer*>* producers, 
		std::vector<ET_Consumer*>* consumers,
		WebSocketSession_Message* subscriptionToken){}


	virtual ET_Producer<WebSocketSession_Message>* getOutputProducer();
	virtual ET_Producer<WebSocketSession_Message>* getInputProducer(string session_id);
	ET_Producer<WebSocketSession_Message>* _outputProducer = nullptr;
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

	struct session_generic_resource
	{
		std::vector<ET_Generic_Producer*>* producers = new std::vector<ET_Generic_Producer*>();
		std::vector<ET_Consumer*>* consumers = new std::vector<ET_Consumer*>();
	};
	map<WebSocket_Session*, session_generic_resource> _session_generic_resources;

};

class WindowInfo_to_WebSocketSession_contentTranslator : public ET_Consumer
{
public:
	WindowInfo_to_WebSocketSession_contentTranslator(ET_Producer<ET_WindowInfo_Content>* source, ET_Producer<WebSocketSession_content>* producer) 
		: _producer(producer), ET_Consumer(source) {};
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
	ET_Logger* _coordinatesLogger;
	ET_Logger* _windowInfoDestination;
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
			: ET_Consumer(echoSource), _echoDestination(echoDestination), _prepend(prepend){};
		void OnReceive(WebSocketSession_Message message);
	protected:
		ET_Producer<WebSocketSession_Message>* _echoDestination;
		string _prepend = "";
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