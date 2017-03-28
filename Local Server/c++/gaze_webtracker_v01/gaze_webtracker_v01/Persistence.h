#pragma once
#include "ET_Interaction.h"
#include <vector>
#include <iostream>
#include <map>
#include <mutex>

#define SERVLET_URI "http://localhost:8888/gaze_analytics"
//#define SERVLET_URI "http://1-dot-mir-project.appspot.com/gaze_analytics"

using namespace std;

class Datasetore_Session_Persister_Sender //one for all sessions
{
public:
	Datasetore_Session_Persister_Sender() {};
	void persist(vector<SessionPersistence_Content> content);
private:
	mutex _mutex;
	virtual void doPersist(vector<SessionPersistence_Content> content);
};

class Datasetore_Session_Persister_Logger : public Datasetore_Session_Persister_Sender //one for all sessions
{
public:
	Datasetore_Session_Persister_Logger() {};
private:
	virtual void doPersist(vector<SessionPersistence_Content> content);
};

class Datasetore_Session_Persister_Buffer //one for every session
{
public:
	Datasetore_Session_Persister_Buffer(Datasetore_Session_Persister_Sender* persister)
		: _persister(persister) {}
	Datasetore_Session_Persister_Buffer(Datasetore_Session_Persister_Sender* persister, int limit) 
		: _limit(limit), _persister(persister){};
	void enqueue(SessionPersistence_Content content);
	void flush();
private:
	int _limit = 0;
	vector<SessionPersistence_Content> _buffer;
	Datasetore_Session_Persister_Sender* _persister;
};

class Datasetore_SessionPersister_Consumer : public ET_Consumer //one for every session
{
public:
	virtual void OnReceive(SessionPersistence_Content content);
	virtual void setPersisterBuffer(Datasetore_Session_Persister_Buffer* persisterBuffer);
	virtual void flushPersisterBuffer();
private:
	Datasetore_Session_Persister_Buffer* _persisterBuffer = nullptr;
};


class Datasetore_Session_Persister //one for all sessions (thread-safe?)
{
public:
	static Datasetore_Session_Persister* Instance();
	Datasetore_SessionPersister_Consumer* getPersisterConsumer(string sessionID);
	void dismissPersisterConsumer(string sessionID); //deprecated
private:
	Datasetore_Session_Persister() {};
	static Datasetore_Session_Persister* _instance;
	//map<string, Datasetore_SessionPersister_Consumer*> _senssion_persisterConsumers;
	//we don't store consumers by sessionID because this allows only one consumer for ALL the services: so only the last service that requested the consumer will be able to use it
	Datasetore_Session_Persister_Sender* _sender;
	mutex _mutex;
};
