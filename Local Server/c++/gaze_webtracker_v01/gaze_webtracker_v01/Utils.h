#pragma once
#include <Windows.h>
#include <string>
#include <thread>
#include <atomic>

std::string TCHAR_to_string(TCHAR* theTHCAR);
std::string chars_to_string(char* theChars);
std::string chars_to_string(char* theChars, unsigned int size);
char* string_to_chars(std::string str);
double getCurrentTimestamp();

class PeriodicFunction
{
public:
	PeriodicFunction();

	~PeriodicFunction();

	void stop();

	void start(std::function<void(void)> func, int milliseconds);

	bool is_running() const noexcept;

private:
	std::atomic<bool> _execute;
	std::thread _thd;
};

std::string File_to_Base64(std::string filepath);
