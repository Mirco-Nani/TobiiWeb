#include "stdafx.h"
#include "POCO_HttpClient.h"
#include "Poco/Net/NetException.h"

#include <algorithm>  // for copy
#include <iterator>
#include <iostream>   // for cout, istream

#include <map>
#include <vector>


void SimpleHttpRequestTest()
{
	URI uri("http://localhost:8888/simpleservlet2?username=user&pass=pass");
	std::string path(uri.getPathAndQuery());

	if (path.empty()) path = "/";

	HTTPClientSession session(uri.getHost(), uri.getPort());
	HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	if (!doRequest(session, request, response))
	{
		std::cerr << "Invalid username or password" << std::endl;
	}
}

void SimpleHttpRequestTest_POST()
{
	URI uri("http://localhost:8888/simpleservlet2");
	std::string path(uri.getPath());
	if (path.empty()) path = "/";

	cout << path << endl;

	HTTPClientSession session(uri.getHost(), uri.getPort());

	HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);

	std::string reqBody("username=user&password=pass");

	HTTPResponse response;

	if (!doRequest(session, reqBody, request, response))
	{
		std::cerr << "something went wrong" << std::endl;
	}
}

bool doRequest(Poco::Net::HTTPClientSession & session, Poco::Net::HTTPRequest & request, Poco::Net::HTTPResponse & response)
{
	session.sendRequest(request);
	std::istream& rs = session.receiveResponse(response);
	std::cout << response.getStatus() << " " << response.getReason() << std::endl;
	if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
	{
		// Print to standard output
		cout << "RECEIVED:" << endl;
		std::copy(std::istream_iterator<char>(rs), std::istream_iterator<char>(), std::ostream_iterator<char>(std::cout) );
		cout << endl;
		return true;
	}
	else
	{
		//it went wrong ?
		return false;
	}
}

bool doRequest(Poco::Net::HTTPClientSession & session, string requestBody, Poco::Net::HTTPRequest & request, Poco::Net::HTTPResponse & response)
{
	session.setKeepAlive(true);
	request.setKeepAlive(true);
	request.setContentType("application/x-www-form-urlencoded");
	request.setContentLength(requestBody.length());

	session.sendRequest(request) << requestBody;
	std::istream& rs = session.receiveResponse(response);
	std::cout << response.getStatus() << " " << response.getReason() << std::endl;
	if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
	{
		// Print to standard output
		cout << "RECEIVED:" << endl;
		string received = "";
		string temp;
		while (std::getline(rs, temp))
		{
			received += temp + "\n";
		}

		cout << received << endl;
		return true;
	}
	else
	{
		//it went wrong ?
		return false;
	}
}
mutex POCO_HttpClient::_mutex;
POCO_HttpClient_response POCO_HttpClient::POST_request(string param_uri)
{
	_mutex.lock();

	URI uri(param_uri);
	string path(uri.getPath());
	if (path.empty()) path = "/";
	string requestBody = uri.getQuery();
	HTTPClientSession session(uri.getHost(), uri.getPort());
	HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	session.setKeepAlive(true);
	request.setKeepAlive(true);
	request.setContentType("application/x-www-form-urlencoded");
	request.setContentLength(requestBody.length());


	string state = "";
	string reason = "";
	string received = "";
	HTTPResponse::HTTPStatus status = HTTPResponse::HTTPStatus::HTTP_NOT_FOUND;

	try 
	{
		session.sendRequest(request) << requestBody;
		std::istream& rs = session.receiveResponse(response);
		status = response.getStatus();
		reason = response.getReason();
		received = "";
		string temp;
		while (getline(rs, temp))
		{
			received += temp + "\n";
		}
		state = "success";
	}
	catch (NetException e)
	{
		state = "exception";
		received = e.displayText();
	}
	catch (...)
	{
		state = "exception";
		received = "exception";
	}
	
	_mutex.unlock();

	return POCO_HttpClient_response(state, received, status, reason);
}


POCO_HttpClient_response POCO_HttpClient::GET_request(string param_uri)
{
	_mutex.lock();

	URI uri(param_uri);
	std::string path(uri.getPathAndQuery());
	if (path.empty()) path = "/";

	HTTPClientSession session(uri.getHost(), uri.getPort());
	HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
	HTTPResponse response;

	string state = "";
	string reason = "";
	string received = "";
	HTTPResponse::HTTPStatus status = HTTPResponse::HTTPStatus::HTTP_NOT_FOUND;
	try
	{
		session.sendRequest(request);
		std::istream& rs = session.receiveResponse(response);
		status = response.getStatus();
		reason = response.getReason();
		received = "";
		string temp;
		while (getline(rs, temp))
		{
			received += temp + "\n";
		}
		state = "success";
	}
	catch (NetException e)
	{
		state = "exception";
		received = e.displayText();
	}
	catch (...)
	{
		state = "exception";
		received = "exception";
	}

	_mutex.unlock();

	return POCO_HttpClient_response(state, received, status, reason);
}
