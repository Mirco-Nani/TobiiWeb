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

#include "stdafx.h"
#include "POCO_WebSocketServer.h"
#include "utils.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/WebSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Format.h"
#include <iostream>

#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"

#include <thread>
#include <chrono>

#include <map>
#include <stdio.h>

#include <mutex>

#include "Windows_sensing.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"



using namespace rapidjson;

using Poco::Net::ServerSocket;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;

using Poco::BasicEvent;
using Poco::Delegate;

using namespace std;

//unsigned int UniqueID::id_count = 0;
UniqueID_getter* UniqueID::_getter;
void UniqueID::Init(UniqueID_getter * getter)
{
	_getter = getter;
}
string UniqueID::get()
{
	if (_getter == nullptr)
	{
		_getter = new UniqueID_getter();
	}
	return _getter->get();//id_count++;
}

string UniqueID_getter::get()
{
	return to_string(_count++);
}




ET_ThreadSafe_Producer<ET_WindowInfo_Content>* POCO_WebSocketServer::_applicationCoordinatesSender = new ET_ThreadSafe_Producer<ET_WindowInfo_Content>();
ET_Producer<ET_JSON_Content>* POCO_WebSocketServer::_applicationResponse_source = nullptr;
ET_Producer<ET_Log>* POCO_WebSocketServer::_log_source = nullptr;


//SERVICES
map<string, WebSocket_Generic_Service*> POCO_WebSocketServer::_services;

void Emit_Log(ET_Log log)
{
	//cout << log.log << endl;
	if (POCO_WebSocketServer::_log_source != nullptr)
	{
		POCO_WebSocketServer::_log_source->Emit(log);
	}
}

void Emit_Log(string s)
{
	Emit_Log(ET_Log(s));
}

void ApplicationResponse_source_Emit(ET_JSON_Content c)
{
	if (POCO_WebSocketServer::_applicationResponse_source != nullptr)
	{
		POCO_WebSocketServer::_applicationResponse_source->Emit(c);
	}
}

class WebSocketRequestHandler : public HTTPRequestHandler
	/// Handle a WebSocket connection.
{
public:
	void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
	{
		Application& app = Application::instance();
		try
		{
			WebSocket* ws = new WebSocket(request, response);
			ws->setReceiveTimeout(Poco::Timespan(1, 0, 0, 0, 0)); //(days, hours, minutes, seconds, microseconds)
																  //for now, a 1-day timeout
			Websocket_Window_Identifier* websocket_Window_Identifier = nullptr;

			//SERVICES
			POCO_WebSocket_Session* session = nullptr;

			bool window_detected = false;
			unsigned long hwnd = 0;
			unsigned long ancestor_hwnd = 0;
			Emit_Log("WebSocket connection established.");
			char buffer[WEBSOCKET_FRAME_BUFFER_SIZE];
			int flags;
			int n;
			do
			{
				POCO_WebSocket_Frame* toSend;
				n = ws->receiveFrame(buffer, sizeof(buffer), flags);

				if ((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_CLOSE)
				{
					Emit_Log("CLOSE FRAME RECEIVED");
					ws->sendFrame(buffer, n, flags);
				}
				else
				{
					WebSocket_Frame_Type frame_type = POCO_WebSocketServer::GetJsonFrameType(buffer, n, flags);
					switch (frame_type)
					{
					case(WebSocket_Frame_Type::UNKNOWN) :
						break;
					case(WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION) :
					{
						Emit_Log("Authentication requested");
						Websocket_Window_Identifier websocket_Window_Identifier = POCO_WebSocketServer::WebPage_Authentication(buffer, n, flags);
						hwnd = websocket_Window_Identifier.hwnd;
						ancestor_hwnd = websocket_Window_Identifier.ancestor_hwnd;
						window_detected = true;
						toSend = POCO_WebSocketServer::GenerateFrame_WebPageAuthenticationDone();
						ws->sendFrame(toSend->content, toSend->length);
					}
						break;
					case(WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION_SUCCESSFUL) :
						Emit_Log("Authentication successful: Session is now open");
						if (window_detected)
						{
							//SERVICES
							buffer[n] = '\0';
							session = POCO_WebSocketServer::WebPage_AuthenticationSuccessful(ws, buffer, hwnd, ancestor_hwnd);
							
						}
						else
						{
							toSend = POCO_WebSocketServer::GenerateFrame_WebPageAuthenticationFailed();
							ws->sendFrame(toSend->content, toSend->length);
						}
						break;
					case(WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION_FAILED) :
						Emit_Log("Authentication failed");
						window_detected = false;
						break;
					case(WebSocket_Frame_Type::WEB_PAGE_SESSION_FAILED) :
						Emit_Log("Session failed");
						window_detected = false;
						//SERVICES
						POCO_WebSocketServer::onWebSocketSession_Close(session);
						break;
					case(WebSocket_Frame_Type::APPLICATION_CONTENT) :
						POCO_WebSocketServer::Emit_ApplicationResponse(session, buffer, n, hwnd, ancestor_hwnd, window_detected);
						break;
					}
				}
			} while (n > 0 || (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);

			POCO_WebSocketServer::onWebSocketConnection_Close(session);

			ws->close();

			Emit_Log("WebSocket connection closed.");
		}
		catch (WebSocketException& exc)
		{
			app.logger().log(exc);
			switch (exc.code())
			{
			case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
				response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
				// fallthrough
			case WebSocket::WS_ERR_NO_HANDSHAKE:
			case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
			case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
				response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
				response.setContentLength(0);
				response.send();
				break;
			}
		}
	}
};


class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request)
	{
		Application& app = Application::instance();
		Emit_Log("Request from "
			+ request.clientAddress().toString()
			+ ": "
			+ request.getMethod()
			+ " "
			+ request.getURI()
			+ " "
			+ request.getVersion());

		for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
		{
			Emit_Log(it->first + ": " + it->second);
		}

		if (request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0)
			return new WebSocketRequestHandler;
		else
		{
			Emit_Log("HTTP page requested: ignored");
		}
	}
};


class WebSocketServer : public Poco::Util::ServerApplication
	/// To test the WebSocketServer, open a websocket to: (http://localhost:6675/).
{
public:
	WebSocketServer() : _helpRequested(false)
	{
	}

	~WebSocketServer()
	{
	}
protected:
	void initialize(Application& self)
	{
		loadConfiguration(); // load default configuration files, if present
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
			Option("help", "h", "display help information on command line arguments")
			.required(false)
			.repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A sample HTTP server supporting the WebSocket protocol.");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			// get parameters from configuration file
			unsigned short port = (unsigned short)config().getInt("WebSocketServer.port", WEBSOCKET_SERVER_STANDARD_PORT);

			// set-up a server socket
			ServerSocket svs(port);
			// set-up a HTTPServer instance
			HTTPServer srv(new RequestHandlerFactory, svs, new HTTPServerParams);
			// start the HTTPServer
			srv.start();
			// wait for CTRL-C or kill
			waitForTerminationRequest();
			// Stop the HTTPServer
			srv.stop();
		}
		return Application::EXIT_OK;
	}

private:
	bool _helpRequested;
};

WebSocketServer webSocketServer;

void Websocket_server_task(int argc, char** argv)
{
	webSocketServer.run(argc, argv);
}

thread start_WebSocketServer(int argc, char** argv)
{
	thread t1(Websocket_server_task, argc, argv);
	return t1;
}


POCO_WebSocketServer* POCO_WebSocketServer::_instance = nullptr;
unsigned short POCO_WebSocketServer::_running = 0;

POCO_WebSocketServer::POCO_WebSocketServer()
{
}

POCO_WebSocketServer::~POCO_WebSocketServer()
{
}

void POCO_WebSocketServer::start_websocketServer(int argc, char** argv)
{
	_webSocketServer_thread = start_WebSocketServer(argc, argv);
}

POCO_WebSocketServer * POCO_WebSocketServer::Instance()
{
	if (_instance == nullptr)_instance = new POCO_WebSocketServer();
	return _instance;
}

void POCO_WebSocketServer::Attach(ET_Producer<ET_WindowInfo_Content>* windowInfoSource)
{
	ET_Consumer::Attach(windowInfoSource);
}

void POCO_WebSocketServer::OnReceive(ET_WindowInfo_Content content)
{
	_applicationCoordinatesSender->Emit(content);
}


void POCO_WebSocketServer::Init(ET_Producer<ET_WindowInfo_Content>* windowInfoSource, ET_Producer<ET_JSON_Content>* applicationResponse_source, ET_Producer<ET_Log>* log_source, UniqueID_getter * uniqueID_getter)
{
	if (uniqueID_getter != nullptr)
	{
		UniqueID::Init(uniqueID_getter);
	}
	else
	{
		UniqueID::Init(new UniqueID_getter());
	}
	this->_log_source = log_source;
	this->_applicationResponse_source = applicationResponse_source;
	Attach(windowInfoSource);
}


void POCO_WebSocketServer::Init(ET_Producer<ET_Log>* log_source, UniqueID_getter * uniqueID_getter)
{
	if (uniqueID_getter != nullptr)
	{
		UniqueID::Init(uniqueID_getter);
	}
	else
	{
		UniqueID::Init(new UniqueID_getter());
	}
	this->_log_source = log_source;
}

void POCO_WebSocketServer::start(int argc, char ** argv)
{
	if (_running)return;

	//SERVICES
	start_services();
	this->start_websocketServer(argc, argv);
	cout << "Websocket server started. waiting for incoming connections.." << endl;
}

void POCO_WebSocketServer::start()
{
	if (_running)return;

	int mock_argc = 1;
	char** mock_argv = new char*[1];
	mock_argv[0] = "";
	this->start(mock_argc, mock_argv);

	_running = 1;
}

void POCO_WebSocketServer::stop()
{
	if (!_running) return;

	//SERVICES
	this->stop_services();

	webSocketServer.terminate();
	this->_webSocketServer_thread.join();

	_running = 0;
}

WebSocket_Frame_Type POCO_WebSocketServer::GetJsonFrameType(char* frame, int frame_length, int flags)
{
	if (frame_length < 2)
	{
		return WebSocket_Frame_Type::UNKNOWN;
	}

	char buffer[WEBSOCKET_FRAME_BUFFER_SIZE];
	strncpy(buffer, frame, frame_length);
	buffer[frame_length] = '\0';

	Document d;
	d.Parse(buffer);

	if (!d.HasMember("type"))
	{
		return WebSocket_Frame_Type::UNKNOWN;
	}
	if (!d["type"].IsString())
	{
		return WebSocket_Frame_Type::UNKNOWN;
	}

	string type = d["type"].GetString();

	return string_to_WebSocketFrameType(type);
}

Websocket_Window_Identifier POCO_WebSocketServer::WebPage_Authentication(char* frame, int frame_length, int flags)
{
	frame[frame_length] = '\0'; //just to be sure

	Document d;
	d.Parse(frame);

	double browser_x = d["browser_x"].GetDouble();
	double browser_y = d["browser_y"].GetDouble();
	double browser_width = d["browser_width"].GetDouble();
	double browser_height = d["browser_height"].GetDouble();
	double viewport_width = d["viewport_width"].GetDouble();
	double viewport_height = d["viewport_height"].GetDouble();

	double possible_border_width = 10; //pixels

	double viewport_x = browser_x + possible_border_width;
	double viewport_y = browser_y + (browser_height - viewport_height) + possible_border_width;

	unsigned long hwnd = GetHwndFrom::ScreenPoint(viewport_x, viewport_y);
	unsigned long ancestor_hwnd = GetHwndFrom::OneOfItsChildren(hwnd);

	Websocket_Window_Identifier result(hwnd, ancestor_hwnd);

	return result;
}

POCO_WebSocket_Frame* POCO_WebSocketServer::GenerateFrame_WebPageAuthenticationDone()
{
	char content[WEBSOCKET_FRAME_BUFFER_SIZE];
	int len = sprintf(content, "{\"type\":\"web_page_authentication_done\"}");
	POCO_WebSocket_Frame result(content, len);
	return &result;
}

POCO_WebSocket_Frame * POCO_WebSocketServer::GenerateFrame_WebPageAuthenticationFailed()
{
	char content[WEBSOCKET_FRAME_BUFFER_SIZE];
	int len = sprintf(content, "{\"type\":\"web_page_authentication_failed\"}");
	POCO_WebSocket_Frame result(content, len);
	return &result;
}



//SERVICES
void POCO_WebSocketServer::RegisterService(WebSocket_Generic_Service* service)
{
	_services[service->getName()] = service;
}


POCO_WebSocket_Session* POCO_WebSocketServer::WebPage_AuthenticationSuccessful(WebSocket* webSocket, char* frame, unsigned long hwnd, unsigned long ancestor_hwnd)
{
	POCO_WebSocket_Session* session = new POCO_WebSocket_Session(  UniqueID::get() , webSocket, hwnd, ancestor_hwnd);

	Document d;
	d.Parse(frame);
	if (d["services"].IsArray())
	{
		for (SizeType i = 0; i < d["services"].Size(); i++)
		{
			if (d["services"][i].IsObject())
			{
				if(d["services"][i]["name"].IsString())
				{
					string serviceName = d["services"][i]["name"].GetString();
					if (_services.find(serviceName) == _services.end())
					{
						Emit_Log("Service \"" + serviceName + "\" requested but not found");
					}
					else
					{
						StringBuffer buffer;
						Writer<StringBuffer> writer(buffer);
						d["services"][i].Accept(writer);

						string content_string = buffer.GetString();
						char* content = (char*)content_string.c_str();
						int size = content_string.length();

						WebSocketSession_clientApplicationContent session_token(serviceName, content, size, hwnd, ancestor_hwnd);

						session->subscribe(_services[serviceName], session_token);
					}
				}
			}
		}
	}
	return session;
}

void POCO_WebSocketServer::onWebSocketConnection_Close(POCO_WebSocket_Session * session)
{
	if (session != nullptr)
	{
		session->close();
		//delete[] session;
		session = nullptr;
	}
}

void POCO_WebSocketServer::onWebSocketSession_Close(POCO_WebSocket_Session * session)
{
	if (session != nullptr)
	{
		session->close();
		//delete[] session;
		session = nullptr;
	}
}

void POCO_WebSocketServer::Emit_ApplicationResponse(POCO_WebSocket_Session* session, char * frame, int length, int hwnd, int ancestor_hwnd, bool window)
{
	if (length < 2)
	{
		Emit_Log("invalid frame received");
	}
	else
	{
		frame[length] = '\0';
		Document d;
		d.Parse(frame);
		string service_name = "unknown";
		if (d.HasMember("service"))
		{
			if (d["service"].IsString())
			{
				service_name = d["service"].GetString();
			}
		}
		if (window)
		{
			session->Emit_Frame(service_name, hwnd, ancestor_hwnd, frame, length); 
		}
		else
		{
			session->Emit_Frame(service_name, frame, length);
		}
	}

}

void POCO_WebSocketServer::start_services()
{
	for (auto iterator : _services)
	{
		iterator.second->start();
	}
}

void POCO_WebSocketServer::stop_services()
{
for (auto iterator : _services)
	{
		iterator.second->stop();
	}
}

WebSocket_Frame_Type string_to_WebSocketFrameType(string s)
{
	if (s == "web_page_authentication") return WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION;
	else if (s == "web_page_authentication_failed") return WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION_FAILED;
	else if (s == "web_page_authentication_successful") return WebSocket_Frame_Type::WEB_PAGE_AUTHENTICATION_SUCCESSFUL;
	else if (s == "web_page_session_failed") return WebSocket_Frame_Type::WEB_PAGE_SESSION_FAILED;
	else if (s == "application_content") return WebSocket_Frame_Type::APPLICATION_CONTENT;
	else return WebSocket_Frame_Type::UNKNOWN;
}

POCO_WebSocket_Application_Response_Logger::POCO_WebSocket_Application_Response_Logger(ET_Producer<ET_JSON_Content>* producer)
{
	this->Attach(producer);
}

void POCO_WebSocket_Application_Response_Logger::OnReceive(ET_JSON_Content content)
{
	cout << chars_to_string(content.json_string) << endl;
}

POCO_WebSocket_Session::POCO_WebSocket_Session(WebSocket * webSocket, unsigned long hwnd, unsigned long ancestor_hwnd)
{
	_webSocket = webSocket;
	set_hwnd(hwnd);
	set_ancestor_hwnd(ancestor_hwnd);
}

POCO_WebSocket_Session::POCO_WebSocket_Session(string id, WebSocket * webSocket, unsigned long hwnd, unsigned long ancestor_hwnd)
{
	_webSocket = webSocket;
	set_hwnd(hwnd);
	set_ancestor_hwnd(ancestor_hwnd);
	set_sessionID(id);
}

void POCO_WebSocket_Session::subscribe(WebSocket_Generic_Service * service)
{
	POCO_WebSocket_ServiceFrame_Consumer* consumer = new POCO_WebSocket_ServiceFrame_Consumer();
	consumer->set_hwnds(_hwnd, _ancestor_hwnd);
	consumer->setWebsocket(_webSocket);
	attach_consumer_to_service(service, consumer);
}

void POCO_WebSocket_Session::subscribe(WebSocket_Generic_Service * service, WebSocketSession_clientApplicationContent subscription_token)
{
	_subscription_tokens.insert_or_assign(service->getName(), subscription_token);
	subscribe(service);
}

void POCO_WebSocket_ServiceFrame_Consumer::setWebsocket(WebSocket * ws)
{
	_websocket = ws;
}

void POCO_WebSocket_ServiceFrame_Consumer::sendContent(WebSocketSession_content content)
{
	_websocket->sendFrame(content.content, content.length);
}