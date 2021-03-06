#pragma once

namespace io {
// FTP file manager
class ftp {
	void*			session;
	void*			connection;
public:
	ftp();
	~ftp();
	void			close();
	bool			createdir(const char* dir);
	bool			connect(const char* server, int port, const char* user, const char* password);
	bool			getdir(char* result, unsigned size);
	bool			getfile(const char* local_file_name, const char* ftr_file_name);
	bool			isexist(const char* ftr_file_name);
	bool			open();
	bool			setdir(const char* result);
};
}
