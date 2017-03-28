#include "stdafx.h"
#include <iostream>
#include "ET_Interaction.h"


ET_Generic_Producer::ET_Generic_Producer()
{
}

ET_Generic_Producer::~ET_Generic_Producer()
{
}

void ET_Generic_Producer::AddDestination(ET_Consumer *destination)
{
	_destinations[_destinations_count] = destination;
	destination->setPosition(_destinations_count);
	_destinations_count += 1;
	
}

void ET_Generic_Producer::RemoveDestination(ET_Consumer *destination)
{
	_destinations.erase(destination->getPosition());
}

void ET_Generic_Producer::SetDestinations(std::map<unsigned int, ET_Consumer*> destinations)
{
	_destinations = destinations;
}

void ET_Generic_Producer::RemoveAllDestinations()
{
	_destinations.clear();
}

ET_Consumer::ET_Consumer(ET_Generic_Producer *source)
{
	Attach(source);
}

ET_Consumer::ET_Consumer()
{
}

ET_Consumer::~ET_Consumer()
{
}

void ET_Consumer::Attach(ET_Generic_Producer *source)
{
	Detach();
	_source = source;
	source->AddDestination(this);
}

void ET_Consumer::Detach()
{
	if(_source != nullptr)
		_source->RemoveDestination(this);
	_source = nullptr;
}

/*
void ET_Consumer::OnReceive(ET_Content content)
{
	std::cout << "content received!" << std::endl;
}

void ET_Consumer::OnReceive(ET_GazeCoordinates_Content content)
{
	printf("received (%.1f,%.1f) at timestamp %.0f\n", content.global_gaze_x, content.global_gaze_y, content.global_gaze_timestamp);
}

void ET_Consumer::OnReceive(ET_WindowInfo_Content content)
{
	std::cout 
		<< content.window_hwnd << " " << content.ancestorWindow_hwnd
		<< " (" << content.window_x << "," << content.window_y << ") "
		<< "(" << content.window_width << " X " << content.window_height << ") " << std::endl;
}

void ET_Consumer::OnReceive(ET_Log content)
{
	std::cout << content.log << std::endl;
}
*/

int ET_Consumer::getPosition()
{
	return _position;
}

void ET_Consumer::setPosition(int pos)
{
	_position = pos;
}

void ET_Consumer::UnhandledTypeError(string type)
{
	cout << "ERROR! unhandled " + type + " subclass. please add the following signature:\n virtual void OnReceive(<YourSubclass> content);\n to class ET_Consumer located in ET_Interaction.h" << endl;
	throw "unhandled " + type + " subclass. please add the following signature:\n virtual void OnReceive(<YourSubclass> content);\n to class ET_Consumer located in ET_Interaction.h";
}

void ET_Logger::OnReceive(ET_Log content)
{
	cout << content.log << endl;
}

void ET_Logger::OnReceive(ET_GazeCoordinates_Content content)
{
	printf("received (%.1f,%.1f) at timestamp %.0f\n", content.global_gaze_x, content.global_gaze_y, content.global_gaze_timestamp);
}

void ET_Logger::OnReceive(ET_WindowInfo_Content content)
{
	std::cout
		<< content.window_hwnd << " " << content.ancestorWindow_hwnd
		<< " (" << content.window_x << "," << content.window_y << ") "
		<< "(" << content.window_width << " X " << content.window_height << ") " << std::endl;
}
