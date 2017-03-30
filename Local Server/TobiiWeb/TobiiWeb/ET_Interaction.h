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

#pragma once

#include <map>
#include <string>
#include <mutex>
#include <thread>
#include <atomic>

#define STANDARD_PERIODIC_PRODUCER_PERIOD 1000

using namespace std;

class ET_Generic_Producer;
class ET_Consumer;

struct ET_Content
{
	bool valid = true;
	ET_Content() { valid = false;  };
};

struct ET_GazeCoordinates_Content : ET_Content
{
	double global_gaze_x;
	double global_gaze_y;
	double global_gaze_timestamp;

	ET_GazeCoordinates_Content() : ET_Content() {};
	ET_GazeCoordinates_Content(double paramX, double paramY, double paramTimestamp)
		: global_gaze_x(paramX), global_gaze_y(paramY), global_gaze_timestamp(paramTimestamp) 
	{};
};

struct ET_WindowInfo_Content : ET_GazeCoordinates_Content
{
	unsigned long window_hwnd;
	double window_x;
	double window_y;
	double window_width;
	double window_height;
	
	unsigned long ancestorWindow_hwnd;
	double ancestorWindow_x;
	double ancestorWindow_y;
	double ancestorWindow_width;
	double ancestorWindow_height;

	ET_WindowInfo_Content() : ET_GazeCoordinates_Content() {};
	ET_WindowInfo_Content(
		double param_gazeX, 
		double param_gazeY, 
		double param_GazeTimestamp, 
		unsigned long param_window_hwnd,
		double param_window_x, 
		double param_window_y,
		double param_window_width,
		double param_window_height,

		unsigned long param_ancestorWindow_hwnd,
		double param_ancestorWindow_x,
		double param_ancestorWindow_y,
		double param_ancestorWindow_width,
		double param_ancestorWindow_height
		
		)
		: ET_GazeCoordinates_Content(param_gazeX, param_gazeY, param_GazeTimestamp), 
		window_hwnd(param_window_hwnd),
		window_x(param_window_x),
		window_y(param_window_y),
		window_width(param_window_width),
		window_height(param_window_height),

		ancestorWindow_hwnd(param_ancestorWindow_hwnd),
		ancestorWindow_x(param_ancestorWindow_x),
		ancestorWindow_y(param_ancestorWindow_y),
		ancestorWindow_width(param_ancestorWindow_width),
		ancestorWindow_height(param_ancestorWindow_height)
	{}
};

struct ET_Log : ET_Content
{
	std::string log;
	ET_Log() : ET_Content(){};
	ET_Log(std::string theLog) : log(theLog) {};
};

struct ET_JSON_Content : ET_Content
{
	char* json_string;
	ET_JSON_Content() : ET_Content() {};
	ET_JSON_Content(char* param_json_string) : json_string(param_json_string) {};
};

struct WebSocketSession_content : ET_Content
{
	char* content;
	int length;
	bool window_needed;
	unsigned long hwnd;
	unsigned long ancestor_hwnd;
	WebSocketSession_content(char* param_content, int param_length, unsigned long param_hwnd, unsigned long param_ancestor_hwnd)
		: content(param_content), hwnd(param_hwnd), ancestor_hwnd(param_ancestor_hwnd), length(param_length), window_needed(true) {};
	WebSocketSession_content(char* param_content, int param_length)
		: content(param_content), length(param_length), window_needed(false) {};
};

struct WebSocketSession_clientApplicationContent : WebSocketSession_content
{
	string service_name;
	WebSocketSession_clientApplicationContent(string param_serviceName, char* param_content, int param_length, unsigned long param_hwnd, unsigned long param_ancestor_hwnd)
		: WebSocketSession_content(param_content,param_length,param_hwnd,param_ancestor_hwnd), service_name(param_serviceName) {};
	WebSocketSession_clientApplicationContent(string param_serviceName,  char* param_content, int param_length)
		: WebSocketSession_content(param_content, param_length), service_name(param_serviceName) {};
};

struct WebSocketSession_Message : WebSocketSession_content
{
	string session_id;
	WebSocketSession_Message(string param_session_id, char* param_content, int param_length, unsigned long param_hwnd, unsigned long param_ancestor_hwnd)
		: WebSocketSession_content(param_content, param_length, param_hwnd, param_ancestor_hwnd), session_id(param_session_id) {};
	WebSocketSession_Message(string param_session_id, char* param_content, int param_length)
		: WebSocketSession_content(param_content, param_length), session_id(param_session_id) {};
	void setContent(string str)
	{
		char* result = new char[str.length() + 1];
		const char* cc = str.c_str();
		for (unsigned int i = 0; i < str.length(); i++)
		{
			result[i] = cc[i];
		}
		result[str.length()] = '\0';
		this->content = result;
		this->length = str.length();
	}
	string getContent()
	{
		char* theChars = this->content;
		int size = this->length;
		string result = "";
		for (int i = 0; i<size; i++)
			result += theChars[i];
		return result;
	}
};


class ET_Generic_Producer
{
public:
	ET_Generic_Producer();
	virtual ~ET_Generic_Producer();

	virtual void AddDestination(ET_Consumer *destination);
	virtual void RemoveDestination(ET_Consumer *destination);
	virtual void SetDestinations(std::map<unsigned int,ET_Consumer*> destination);
	virtual void RemoveAllDestinations();

protected:
	std::map<unsigned int,ET_Consumer*> _destinations;
	unsigned int _destinations_count = 0;
};

template <typename T> class ET_Producer : public ET_Generic_Producer
{
public:
	virtual void Emit(T content)
	{
		for (auto it : _destinations)
		{
			it.second->OnReceive(content);
		}
	};
};

template <typename T> class ET_Periodic_Producer //: public ET_Producer<T>
{
public:
	ET_Periodic_Producer();

	virtual ET_Producer<T>* getProducer();

	virtual void stop();

	virtual void start(int period_in_milliseconds = STANDARD_PERIODIC_PRODUCER_PERIOD);

	virtual bool is_running() const noexcept;


protected:
	std::atomic<bool> _execute;
	std::thread _thd;
	ET_Producer<T>* _producer = nullptr;


	virtual T Produce_Content() { return T(); };

};


template <typename T> class ET_ThreadSafe_Producer : public ET_Generic_Producer
{
public:
	virtual void AddDestination(ET_Consumer *destination);
	virtual void RemoveDestination(ET_Consumer *destination);
	virtual void RemoveAllDestinations();

	virtual void Emit(T content)
	{
		for (auto it : _destinations)
		{
			it.second->OnReceive(content);
		}
	};

private:
	mutex _mutex;
};

class ET_Consumer
{
public:
	ET_Consumer(ET_Generic_Producer *source);
	ET_Consumer();
	virtual ~ET_Consumer();

	virtual void Attach(ET_Generic_Producer *source);
	virtual void Detach();

	virtual void OnReceive(ET_Content content){ UnhandledTypeError("ET_Content"); };
	virtual void OnReceive(ET_GazeCoordinates_Content content){ UnhandledTypeError("ET_GazeCoordinates_Content"); };
	virtual void OnReceive(ET_WindowInfo_Content content) { UnhandledTypeError("ET_WindowInfo_Content"); };
	virtual void OnReceive(ET_Log content) { UnhandledTypeError("ET_Log"); };
	virtual void OnReceive(ET_JSON_Content) { UnhandledTypeError("ET_JSON_Content"); };
	virtual void OnReceive(WebSocketSession_content content) { UnhandledTypeError("WebSocketSession_content"); };
	virtual void OnReceive(WebSocketSession_clientApplicationContent content) { UnhandledTypeError("WebSocketSession_clientApplicationContent"); };
	virtual void OnReceive(WebSocketSession_Message content) { UnhandledTypeError("WebSocketSession_Message"); };
	
	int getPosition();
	void setPosition(int pos);

protected:
	int _position;
	ET_Generic_Producer *_source = nullptr;

private:
	void UnhandledTypeError(string type);
};

class ET_Logger : public ET_Consumer
{
public:
	ET_Logger(ET_Generic_Producer* source) : ET_Consumer(source) {};
	virtual void OnReceive(ET_Log content);
	virtual void OnReceive(ET_GazeCoordinates_Content content);
	virtual void OnReceive(ET_WindowInfo_Content content);
};


template<typename T>
inline void ET_ThreadSafe_Producer<T>::AddDestination(ET_Consumer * destination)
{
	_mutex.lock();
	_destinations[_destinations_count] = destination;
	destination->setPosition(_destinations_count);
	_destinations_count+=1;
	_mutex.unlock();
}

template<typename T>
inline void ET_ThreadSafe_Producer<T>::RemoveDestination(ET_Consumer* destination)
{
	_mutex.lock();
	_destinations.erase(destination->getPosition());
	_mutex.unlock();
}
template<typename T>
inline void ET_ThreadSafe_Producer<T>::RemoveAllDestinations()
{
	_mutex.lock();
	_destinations.clear();
	_mutex.unlock();
}

template<typename T>
inline ET_Periodic_Producer<T>::ET_Periodic_Producer()
{
	_producer = new ET_Producer<T>();
}

template<typename T>
inline void ET_Periodic_Producer<T>::stop()
{
	_execute.store(false, std::memory_order_release);
	if (_thd.joinable())
		_thd.join();
}

template<typename T>
inline void ET_Periodic_Producer<T>::start(int period_in_milliseconds)
{
	if (_execute.load(std::memory_order_acquire)) {
		stop();
	};
	_execute.store(true, std::memory_order_release);
	_thd = std::thread([this, period_in_milliseconds]()
	{
		while (_execute.load(std::memory_order_acquire)) {
			this->_producer->Emit(this->Produce_Content());
			//cout << "sleep" << endl;
			std::this_thread::sleep_for(
				std::chrono::milliseconds(period_in_milliseconds));
		}
	});
}

template<typename T>
inline bool ET_Periodic_Producer<T>::is_running() const noexcept
{
	return ( _execute.load(std::memory_order_acquire) && _thd.joinable() );
}

template<typename T>
inline ET_Producer<T>* ET_Periodic_Producer<T>::getProducer()
{
	return _producer;
}
