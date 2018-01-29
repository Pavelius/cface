#include "win.h"
#include "io_ftp.h"

io::ftp::ftp() : session(0), connection(0) {
}

io::ftp::~ftp() {
}

bool io::ftp::open() {
	if(session)
		return true;
	session = InternetOpenA("FTP", 0,
		0, 0, 0);
	return session != 0;
}

void io::ftp::close() {
	if(connection) {
		InternetCloseHandle(connection);
		connection = 0;
	}
	if(session) {
		InternetCloseHandle(session);
		session = 0;
	}
}

bool io::ftp::connect(const char* server, int port, const char* user, const char* password) {
	if(connection)
		return true;
	connection = InternetConnectA(session, server, port, user, password,
		INTERNET_SERVICE_FTP, 0, 0);
	return connection != 0;
}


bool io::ftp::setdir(const char* result) {
	return FtpSetCurrentDirectoryA(connection, result) != 0;
}

bool io::ftp::getdir(char* result, unsigned size) {
	return FtpGetCurrentDirectoryA(connection, result, &size) != 0;
}

bool io::ftp::createdir(const char* name) {
	return FtpCreateDirectoryA(connection, name) != 0;
}