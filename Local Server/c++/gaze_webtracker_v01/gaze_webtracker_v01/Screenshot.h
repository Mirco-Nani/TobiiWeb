#pragma once
#include "thread"
#include <atomic>
#include "ET_Interaction.h"
#include "Utils.h"
#include <iostream>

using namespace std;


class ScreenshotTaker : public ET_Consumer_Of<ScreenshotRequest_content>
{
public:
	ScreenshotTaker(ET_Producer<ScreenshotRequest_content>* source, ET_Producer<ScreenshotMetadata_content>* screenshotMetadata_source = nullptr);
	virtual void OnReceive(ScreenshotRequest_content content);
private:
	ET_Producer<ScreenshotMetadata_content>* _screenshotMetadata_source;
};

class Periodic_WindowScreenshotRequestor : public ET_Periodic_Producer<ScreenshotRequest_content>
{
public:
	Periodic_WindowScreenshotRequestor(unsigned long hwnd, string unique_id);
protected:
	unsigned long _hwnd;
	string _unique_id;
	unsigned int _screens_count = 0;

	virtual ScreenshotRequest_content Produce_Content();
};

class ScreenshotMetadata_Logger : public ET_Consumer
{
public:
	ScreenshotMetadata_Logger(ET_Generic_Producer* producer);
	virtual void OnReceive(ScreenshotMetadata_content content);
};