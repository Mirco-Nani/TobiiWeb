#pragma once
#include "ET_Interaction.h"

class GetHwndFrom
{
public:
	static unsigned long ScreenPoint(long x, long y);
	static unsigned long OneOfItsChildren(unsigned long hwnd); // returns the top level window of hwnd
};

class WindowPerceiver : ET_Consumer
{
public:
	WindowPerceiver(ET_Producer<ET_GazeCoordinates_Content>* coordinatesSource);
	void Init(ET_Producer<ET_WindowInfo_Content>* windowInfoSource = new ET_Producer<ET_WindowInfo_Content>());
	ET_Producer<ET_WindowInfo_Content>* getWindowInfoSource();
	virtual void OnReceive(ET_GazeCoordinates_Content coords);

protected:
	ET_Producer<ET_WindowInfo_Content>* _windowInfoSource = nullptr;

	void EmitWindowInfo(ET_WindowInfo_Content windowInfo);
	void Error_No_WindowInfoSource();
	void Perceive_Window(ET_GazeCoordinates_Content coords);
	
};

class ScreenshotPrinter{
public:
	static bool TakeScreenshotOfWindow(unsigned long hwnd_id, string file_destination);
};