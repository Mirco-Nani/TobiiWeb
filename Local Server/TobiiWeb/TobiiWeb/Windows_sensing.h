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