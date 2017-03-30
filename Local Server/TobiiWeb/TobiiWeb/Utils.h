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
