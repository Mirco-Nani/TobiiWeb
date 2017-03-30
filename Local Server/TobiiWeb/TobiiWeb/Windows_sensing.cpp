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
#include "Windows_sensing.h"
#include "Utils.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include "ScreenImage.h"

using namespace std;

HWND to_hwnd(unsigned long casted_hwnd)
{
	return (HWND)casted_hwnd;
}

WindowPerceiver::WindowPerceiver(ET_Producer<ET_GazeCoordinates_Content>* coordinatesSource)
	:ET_Consumer(coordinatesSource)
{
}

void WindowPerceiver::Init(ET_Producer<ET_WindowInfo_Content>* windowInfoSource)
{
	_windowInfoSource = windowInfoSource;
}

ET_Producer<ET_WindowInfo_Content>* WindowPerceiver::getWindowInfoSource()
{
	if (_windowInfoSource != nullptr)
		return _windowInfoSource;
	else
		Error_No_WindowInfoSource();

	return nullptr;
}

void WindowPerceiver::OnReceive(ET_GazeCoordinates_Content coords)
{
	this->Perceive_Window(coords);
}

void WindowPerceiver::EmitWindowInfo(ET_WindowInfo_Content windowInfo)
{
	if (_windowInfoSource != nullptr)
		_windowInfoSource->Emit(windowInfo);
	else
		Error_No_WindowInfoSource();
}

void WindowPerceiver::Error_No_WindowInfoSource()
{
	throw "WindowPerceiver: No source producer of ET_WindowInfo_Content found, did you forget to call Init() function first?";
}

void print_hwnd(HWND hwnd)
{
	TCHAR* buffer = new TCHAR[4096];
	memset(buffer, 0, (4096)* sizeof(TCHAR));
	GetWindowText(hwnd, buffer, 4096);
	string windowText = TCHAR_to_string(buffer);

	cout << "title: " << windowText << endl;

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	cout << "(" << windowRect.left << "," << windowRect.top << ") "
		<< (windowRect.right - windowRect.left) << "X" << (windowRect.bottom - windowRect.top) << endl;
}

void WindowPerceiver::Perceive_Window(ET_GazeCoordinates_Content coords)
{
	POINT gazePoint;
	gazePoint.x = coords.global_gaze_x;
	gazePoint.y = coords.global_gaze_y;
	HWND hwnd = WindowFromPoint(gazePoint);
	HWND ancestor_hwnd = GetAncestor(hwnd, GA_ROOT);

	RECT windowRect;
	GetWindowRect(hwnd, &windowRect);

	RECT ancestor_windowRect;
	GetWindowRect(ancestor_hwnd, &ancestor_windowRect);

	unsigned long hwnd_id = (unsigned long)hwnd;
	unsigned long ancestor_hwnd_id = (unsigned long)ancestor_hwnd;

	this->EmitWindowInfo(ET_WindowInfo_Content(
		coords.global_gaze_x,
		coords.global_gaze_y,
		coords.global_gaze_timestamp,
		hwnd_id,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		ancestor_hwnd_id,
		ancestor_windowRect.left,
		ancestor_windowRect.top,
		ancestor_windowRect.right - ancestor_windowRect.left,
		ancestor_windowRect.bottom - ancestor_windowRect.top
		));
}

unsigned long GetHwndFrom::ScreenPoint(long x, long y)
{
	POINT p;
	p.x = x;
	p.y = y;
	HWND result = WindowFromPoint(p);
	return (unsigned long)result;
}

unsigned long GetHwndFrom::OneOfItsChildren(unsigned long hwnd_id)
{
	HWND hwnd = to_hwnd(hwnd_id);
	HWND result = GetAncestor(hwnd, GA_ROOT);
	return (unsigned long)result;

}

bool ScreenshotPrinter::TakeScreenshotOfWindow(unsigned long hwnd_id, string file_destination)
{
	bool result = true;

	HWND hwnd = to_hwnd(hwnd_id);

	CScreenImage* img = new CScreenImage();
	if (img->CaptureWindow(hwnd))
	{
		CString filename = file_destination.c_str();
		img->Save(filename);
		img->Destroy();

		result = true;
	}
	else
	{
		result = false;
		cout << "image not captured!" << endl;
	}
	delete img;

	return result;
}
