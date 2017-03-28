#pragma once
#include <map>
#include <thread>
#include "ET_Interaction.h"
#include "Poco/Net/WebSocket.h"
#include "WebSocket_Services.h"

using Poco::Net::WebSocket;

#define WEBSOCKET_FRAME_BUFFER_SIZE 1024*256
#define WEBSOCKET_SERVER_STANDARD_PORT 6675


#define HTTP_ID_URI "http://localhost:8888/gaze_analytics?action=session_id"
//#define HTTP_ID_URI "http://1-dot-mir-project.appspot.com/gaze_analytics?action=session_id"

/* -------------------------- Session's Unique ID classes -------------------------- */
class UniqueID_getter
{
public:
	virtual string get();
private:
	unsigned long _count = 0;
};

class UniqueID_HTTPgetter : public UniqueID_getter
{
public:
	virtual string get();
};

class UniqueID
{
public:
	static void Init(UniqueID_getter* getter);
	static string get();
private:
	//static unsigned int id_count;
	static UniqueID_getter* _getter;
};


/* -------------------------- Websocket Server's structs and utilities -------------------------- */
enum WebSocket_Frame_Type
{
	UNKNOWN,
	WEB_PAGE_AUTHENTICATION, //REQUEST
	WEB_PAGE_AUTHENTICATION_FAILED,
	WEB_PAGE_AUTHENTICATION_SUCCESSFUL,
	WEB_PAGE_SESSION_FAILED,
	APPLICATION_CONTENT
};

WebSocket_Frame_Type string_to_WebSocketFrameType(string s);

struct Websocket_Window_Identifier
{
	unsigned long hwnd;
	unsigned long ancestor_hwnd;

	Websocket_Window_Identifier(
		unsigned long param_hwnd,
		unsigned long param_ancestor_hwnd
		)
		:
		hwnd(param_hwnd),
		ancestor_hwnd(param_ancestor_hwnd)
	{};
	bool operator==(const Websocket_Window_Identifier other)
	{
		return (this->hwnd == other.hwnd) && (this->ancestor_hwnd == other.ancestor_hwnd);
	}
};

struct POCO_WebSocket_Frame
{
	char* content;
	int length;
	POCO_WebSocket_Frame(char* param_content, int param_length)
		: content(param_content), length(param_length) {}
};


class POCO_WebSocket_Application_Response_Logger : public ET_Consumer
{
public:
	POCO_WebSocket_Application_Response_Logger() : ET_Consumer() {};
	POCO_WebSocket_Application_Response_Logger(ET_Producer<ET_JSON_Content>* producer);
	virtual void OnReceive(ET_JSON_Content content);

};


/* -------------------------- Websocket Server's Services -------------------------- */
class POCO_WebSocket_ServiceFrame_Consumer : public WebSocket_ServiceFrame_Consumer
{
public:
	virtual void setWebsocket(WebSocket* ws);

protected:
	virtual void sendContent(WebSocketSession_content content);

	WebSocket* _websocket;
};


class POCO_WebSocket_Session : public WebSocket_Session
{
	public:
		POCO_WebSocket_Session(WebSocket* webSocket, unsigned long hwnd, unsigned long ancestor_hwnd);
		POCO_WebSocket_Session(string id, WebSocket* webSocket, unsigned long hwnd, unsigned long ancestor_hwnd);
		virtual void subscribe(WebSocket_Generic_Service* service);
		virtual void subscribe(WebSocket_Generic_Service* service, WebSocketSession_clientApplicationContent subscription_token);;
	protected:
		WebSocket* _webSocket;
};


class POCO_WebSocketServer : public ET_Consumer
{
public:

	static POCO_WebSocketServer* Instance();

	virtual void Attach(ET_Producer<ET_WindowInfo_Content>* windowInfoSource);
	virtual void OnReceive(ET_WindowInfo_Content content);

	virtual void Init(ET_Producer<ET_WindowInfo_Content>* windowInfoSource, ET_Producer<ET_JSON_Content>* applicationResponse_source = nullptr, ET_Producer<ET_Log>* log_source = nullptr, UniqueID_getter * uniqueID_getter = nullptr);
	virtual void Init(ET_Producer<ET_Log>* log_source = nullptr, UniqueID_getter * uniqueID_getter = nullptr);

	virtual void start(int argc, char** argv);
	virtual void start();

	virtual void stop();


	static WebSocket_Frame_Type GetJsonFrameType(char* frame, int frame_lenght, int flags);

	static Websocket_Window_Identifier WebPage_Authentication(char* frame, int frame_length, int flags);


	static POCO_WebSocket_Frame* GenerateFrame_WebPageAuthenticationDone();
	static POCO_WebSocket_Frame* GenerateFrame_WebPageAuthenticationFailed();
	

	static ET_Producer<ET_JSON_Content>* _applicationResponse_source;
	static ET_Producer<ET_Log>* _log_source;


	//SERVICES
	void RegisterService(WebSocket_Generic_Service* service);
	
	static POCO_WebSocket_Session* WebPage_AuthenticationSuccessful(WebSocket* webSocket, char* frame, unsigned long hwnd, unsigned long ancestor_hwnd);
	static void onWebSocketConnection_Close(POCO_WebSocket_Session* session);
	static void onWebSocketSession_Close(POCO_WebSocket_Session* session);

	static void Emit_ApplicationResponse(POCO_WebSocket_Session* session, char* frame, int length, int hwnd, int ancestor_hwnd, bool window);

protected:
	static ET_ThreadSafe_Producer<ET_WindowInfo_Content>* _applicationCoordinatesSender;
	static POCO_WebSocketServer* _instance;
	static unsigned short _running;

	thread _webSocketServer_thread;

	POCO_WebSocketServer();
	virtual ~POCO_WebSocketServer();
	virtual void start_websocketServer(int argc, char** argv);


	//SERVICES
	static map<string, WebSocket_Generic_Service*> _services;
	
	virtual void start_services();
	virtual void stop_services();
	
};