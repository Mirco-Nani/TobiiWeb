#include "stdafx.h"
#include "Screenshot.h"
#include "Windows_sensing.h"

ScreenshotTaker::ScreenshotTaker(ET_Producer<ScreenshotRequest_content>* source, ET_Producer<ScreenshotMetadata_content>* screenshotMetadata_source)
	: ET_Consumer_Of<ScreenshotRequest_content>(source)
{
	this->_screenshotMetadata_source = screenshotMetadata_source;
}

void ScreenshotTaker::OnReceive(ScreenshotRequest_content content)
{
	if (ScreenshotPrinter::TakeScreenshotOfWindow(content.hwnd, content.location_path))
	{
		if(this->_screenshotMetadata_source != nullptr)
		{
			_screenshotMetadata_source->Emit(ScreenshotMetadata_content(content.location_path, getCurrentTimestamp()));
		}
	}
}

Periodic_WindowScreenshotRequestor::Periodic_WindowScreenshotRequestor(unsigned long hwnd, string unique_id)
	: _hwnd(hwnd), _unique_id(unique_id), ET_Periodic_Producer<ScreenshotRequest_content>()
{
}

ScreenshotRequest_content Periodic_WindowScreenshotRequestor::Produce_Content()
{
	_screens_count += 1;
	return ScreenshotRequest_content(_hwnd, "../media/" + _unique_id + "_" + to_string(_screens_count) + ".jpg");
}

ScreenshotMetadata_Logger::ScreenshotMetadata_Logger(ET_Generic_Producer * producer)
	: ET_Consumer(producer)
{
}

void ScreenshotMetadata_Logger::OnReceive(ScreenshotMetadata_content content)
{
	cout << content.location_path << " at " << to_string(content.timestamp) << endl;
}
