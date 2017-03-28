#pragma once

#include <string>
#include <mutex>

#include "Poco/Exception.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Path.h"
#include "Poco/URI.h"

using namespace Poco;
using namespace Net;
using namespace std;

void SimpleHttpRequestTest();
void SimpleHttpRequestTest_POST();
bool doRequest(Poco::Net::HTTPClientSession& session, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response);
bool doRequest(Poco::Net::HTTPClientSession& session, std::string requestBody, Poco::Net::HTTPRequest& request, Poco::Net::HTTPResponse& response);

struct POCO_HttpClient_response 
{
	string response;
	string state;
	HTTPResponse::HTTPStatus status;
	string reason;

	POCO_HttpClient_response(string param_state, string param_response, HTTPResponse::HTTPStatus param_status, string param_reason)
		: state(param_state), response(param_response), status(param_status), reason(param_reason) {}
};
class POCO_HttpClient
{
public:
	static POCO_HttpClient_response POST_request(string uri);
	static POCO_HttpClient_response GET_request(string uri);
private:
	static  mutex _mutex;
};