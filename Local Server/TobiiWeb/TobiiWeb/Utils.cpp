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

#include <chrono>
#include "stdafx.h"
#include "Utils.h"
//#include "Poco/Crypto/Cipher.h"
#include <stdio.h>

#include "Poco/Base64Encoder.h"
#include "Poco/FileStream.h"
#include <sstream>
#include <fstream>




using namespace std::chrono;

std::string TCHAR_to_string(TCHAR * theTHCAR)
{
	std::string result = "";
	for (int i = 0; theTHCAR[i] != '\0'; i++)
		result += (char)theTHCAR[i];
	return result;
}
std::string chars_to_string(char * theChars)
{
	std::string result = "";
	for (int i = 0; theChars[i] != '\0'; i++)
		result += theChars[i];
	return result;
}
std::string chars_to_string(char * theChars, unsigned int size)
{
	std::string result = "";
	for (unsigned int i = 0; i<size; i++)
		result += theChars[i];
	return result;
}

char * string_to_chars(std::string str)
{
	char* result = new char[str.length() + 1];
	const char* cc = str.c_str();
	for (int i = 0; i < str.length(); i++)
	{
		result[i] = cc[i];
	}
	result[str.length()] = '\0';
	return result;
}



double getCurrentTimestamp()
{
	milliseconds ms = duration_cast< milliseconds >(
		system_clock::now().time_since_epoch()
		);
	
	return ms.count();
}

PeriodicFunction::PeriodicFunction()
	:_execute(false)
{}

PeriodicFunction::~PeriodicFunction() {
	if (_execute.load(std::memory_order_acquire)) {
		stop();
	};
}

void PeriodicFunction::stop()
{
	_execute.store(false, std::memory_order_release);
	if (_thd.joinable())
		_thd.join();
}

void PeriodicFunction::start(std::function<void(void)> func, int milliseconds)
{
	if (_execute.load(std::memory_order_acquire)) {
		stop();
	};
	_execute.store(true, std::memory_order_release);
	_thd = std::thread([this, milliseconds, func]()
	{
		while (_execute.load(std::memory_order_acquire)) {
			func();
			std::this_thread::sleep_for(
				std::chrono::milliseconds(milliseconds));
		}
	});
}

bool PeriodicFunction::is_running() const noexcept {
	return (_execute.load(std::memory_order_acquire) &&
		_thd.joinable());
}


std::string File_to_Base64(std::string filepath)
{
	std::ifstream is(filepath, std::ifstream::binary);
	is.seekg(0, is.end);
	int length = is.tellg();

	if (length < 0) 
	{
		return "";
	}

	is.seekg(0, is.beg);

	//std::cout << length << std::endl;

	char * buffer = new char[length];
	is.read(buffer, length);

	std::string read;

	for (int i = 0; i < length; i++)
	{
		read += buffer[i];
	}
	
	std::ostringstream ostr;
	Poco::Base64Encoder encoder(ostr);

	encoder << read;
	encoder.flush();
	encoder.close();

	ostr.flush();
	is.close();
	delete[] buffer;
	
	return ostr.str();
}

/*
std::string FileImage_to_Base64(std::string filepath)
{
	Poco::FileOutputStream sink("encrypted.dat");
	CryptoOutputStream encryptor(sink, pCipher->createEncryptor());

	Poco::FileInputStream source("source.txt");
	Poco::StreamCopier::copyStream(source, encryptor);

	// Always close output streams to flush all internal buffers
	encryptor.close();
	sink.close();
}
*/