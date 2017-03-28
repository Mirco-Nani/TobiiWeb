#include "stdafx.h"
#include "Persistence.h"
#include "POCO_HttpClient.h"

#include <thread>

Datasetore_Session_Persister* Datasetore_Session_Persister::_instance = nullptr;
Datasetore_Session_Persister * Datasetore_Session_Persister::Instance()
{
	if (_instance == nullptr)
	{
		_instance = new Datasetore_Session_Persister();
		_instance->_sender = new Datasetore_Session_Persister_Sender();
	}
	return _instance;
}

Datasetore_SessionPersister_Consumer * Datasetore_Session_Persister::getPersisterConsumer(string sessionID)
{
	Datasetore_SessionPersister_Consumer* persisterConsumer = new Datasetore_SessionPersister_Consumer();
	Datasetore_Session_Persister_Buffer* buffer = new Datasetore_Session_Persister_Buffer(_sender);
	persisterConsumer->setPersisterBuffer(buffer);
	return persisterConsumer;
}

void Datasetore_Session_Persister::dismissPersisterConsumer(string sessionID)
{
}

void Datasetore_Session_Persister_Buffer::enqueue(SessionPersistence_Content content)
{
	_buffer.push_back(content);
	if (_limit > 0)
	{
		if (_buffer.size() >= (unsigned int)_limit)
		{
			this->flush();
		}
	}
}

void Datasetore_Session_Persister_Buffer::flush()
{
	vector<SessionPersistence_Content> clone(_buffer);
	_persister->persist(clone);
	_buffer.clear();
}

void Datasetore_SessionPersister_Consumer::OnReceive(SessionPersistence_Content content)
{
	if (_persisterBuffer != nullptr)
	{
		_persisterBuffer->enqueue(content);
	}
}

void Datasetore_SessionPersister_Consumer::setPersisterBuffer(Datasetore_Session_Persister_Buffer * persisterBuffer)
{
	_persisterBuffer = persisterBuffer;
}

void Datasetore_SessionPersister_Consumer::flushPersisterBuffer()
{
	if (_persisterBuffer != nullptr)
	{
		_persisterBuffer->flush();
	}
}


void Datasetore_Session_Persister_Logger::doPersist(vector<SessionPersistence_Content> content)
{
	for (auto iterator : content)
	{
		cout << iterator.session_id << ":" << endl << iterator.JSON_content << endl << endl;
	}
}

void Datasetore_Session_Persister_Sender::persist(vector<SessionPersistence_Content> content)
{
	_mutex.lock();
	doPersist(content);
	_mutex.unlock();
}

void Datasetore_Session_Persister_Sender::doPersist(vector<SessionPersistence_Content> content)
{
	//this executes in the WebSocket's thread, so it won't block the main thread waiting for the HTTP server.
	//however, this could block the WebSocket's thread, so it's better to call it only when the websocket connection closes.

	string uri = SERVLET_URI;
	string json = "[";
	for (auto iterator : content)
	{
		json += iterator.JSON_content + ",";
	}
	json = json.substr(0, json.size() - 1);
	json += "]";
	if (json.length() < 2) return;
	string query = "?action=store_session_contents&content=" + json;
	uri += query;
	printf("%s\n\n", uri.c_str());
	POCO_HttpClient_response resp = POCO_HttpClient::POST_request(uri);
	printf("%s\n\n\n", resp.response.c_str());
}
