#include "stdafx.h"
#include "POCO_FtpClient.h"

#include "Poco/Net/FTPClientSession.h"
#include <iostream>
#include <fstream>

using namespace std;

void POCO_FtpClient::test()
{
	Poco::Timespan time(0, 0, 1, 0, 0);
	string remote_dir = "/media/";
	string filename = "test.txt";
	string host = "ftp.mirdevelopment.altervista.org";
	string user = "mirdevelopment";
	string password = "cunrapeko93";

	string remote_filename = "img.bmp";
	string local_filename = "../media/1_1.jpg";

	cout << "reading file.." << endl;

	ifstream infile(local_filename, std::ifstream::binary);

	// get size of file
	infile.seekg(0, infile.end);
	long size = infile.tellg();
	infile.seekg(0);

	// allocate memory for file content
	char* buffer = new char[size];

	// read content of infile
	infile.read(buffer, size);

	Poco::Net::FTPClientSession session(host);
	session.setTimeout(time);
	session.login(user, password);
	session.setWorkingDirectory(remote_dir);

	cout << "begin upload..." << endl;

	ostream& ostr = session.beginUpload(remote_filename);
	ostr.write(buffer,size);
	session.endUpload();

	cout << "upload complete" << endl;

	cout << "cleaning up..." << endl;

	// release dynamically-allocated memory
	delete[] buffer;

	infile.close();

	session.close();

	cout << "done!" << endl;
}
