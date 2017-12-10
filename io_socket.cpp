#include "command.h"
#include "crt.h"
#include "io.h"

// POSIX api
enum posix_errors {
	AllOK,
};

#define AF_INET		2		/* Internet IP Protocol 	*/
#define AF_IPX		4		/* Novell IPX 			*/
#define AF_INET6	10		/* IP version 6			*/

#define IPPROTO_TCP 6

#define MSG_WAITALL 0x8

#define SOCAPI __stdcall

struct addrinfo {
	int             		flags;
	int             		family;
	int             		socktype;
	int             		protocol;
	int						addrlen;
	char*					canonname;
	io::address*			addr;
	addrinfo*				next;
};

extern "C" int SOCAPI		accept(int socket, io::address* address, int *address_len);
extern "C" int SOCAPI 		bind(int socket, const io::address* address, int address_len);
extern "C" int SOCAPI 		connect(int socket, const io::address* address, int address_len);
extern "C" int SOCAPI 		closesocket(int socket);
extern "C" void	SOCAPI		freeaddrinfo(addrinfo *ai);
extern "C" int SOCAPI 		getnameinfo(const io::address* sa, int salen, char* node, int nodelen, char* service, int servicelen, int flags);
extern "C" int SOCAPI		getaddrinfo(const char* url, const char* service_name, const addrinfo* hints, addrinfo** result);
extern "C" unsigned short SOCAPI htons(unsigned short hostshort); // Translate port unsigned int to byte order of net protocols
extern "C" unsigned SOCAPI	inet_addr(const char* ip);
extern "C" int SOCAPI 		listen(int socket, int backlog);
extern "C" int SOCAPI 		recv(int socket, void *buffer, int length, int flags);
extern "C" int SOCAPI 		send(int socket, const void *message, int length, int flags);
extern "C" int SOCAPI 		shutdown(int socket, int how);
extern "C" int SOCAPI 		socket(int domain, int type, int protocol);

void io::address::clear() {
	memset(this, 0, sizeof(address));
}

bool io::address::parse(const char* url, const char* service_name) {
	clear();
	addrinfo hint = {0};
	addrinfo* result = 0;
	hint.socktype = TCP;
	hint.protocol = IPPROTO_TCP;
	int e = getaddrinfo(url, service_name, &hint, &result);
	if(e != AllOK)
		return false;
	for(addrinfo* p = result; p; p = p->next) {
		if(p->addrlen != sizeof(address))
			continue;
		memcpy(&family, p->addr, sizeof(address));
		break;
	}
	freeaddrinfo(result);
	return true;
}

bool io::address::tostring(char* node, int node_len, char* service, int service_len) {
	int e = getnameinfo(this, sizeof(address), node, node_len, service, service_len, 0);
	if(e != AllOK)
		return false;
	return true;
}

io::socket::socket() : s(0) {
	clear();
}

io::socket::~socket() {
	if(s) {
		closesocket(s);
		s = 0;
	}
}

bool io::socket::create(protocols type) {
	clear();
	s = ::socket(AF_INET, type, 0);
	if(s < 0) {
		s = 0;
		return false;
	}
	return true;
}

bool io::socket::bind() {
	return ::bind(s, this, sizeof(address)) == AllOK;
}

bool io::socket::connect() {
	return ::connect(s, this, sizeof(address)) == AllOK;
}

void io::socket::listen(int backlog) {
	::listen(s, backlog);
}

void io::socket::accept(socket& e) {
	int address_size = sizeof(address);
	e.s = ::accept(s, &e, &address_size);
}

int io::socket::read(void* result, int count) {
	return recv(s, result, count, MSG_WAITALL);
}

int io::socket::write(const void* result, int count) {
	return send(s, result, count, 0);
}

#define WSADESCRIPTION_LEN  256
#define WSASYS_STATUS_LEN   128
struct WSAData {
	unsigned short			wVersion;
	unsigned short			wHighVersion;
	char					szDescription[WSADESCRIPTION_LEN + 1];
	char					szSystemStatus[WSASYS_STATUS_LEN + 1];
	unsigned short			iMaxSockets;
	unsigned short			iMaxUdpDg;
	const char*				lpVendorInfo;
};

extern "C" int SOCAPI 		WSACleanup(void);
extern "C" int SOCAPI 		WSAStartup(unsigned short wVersionRequested, WSAData* lpWSAData);

static void deinitialize() {
	WSACleanup();
}

COMMAND(app_initialize) {
	WSAData wsa = {0};
	int e = WSAStartup(0x0202, &wsa);
	if(e != AllOK)
		return;
	atexit(deinitialize);
}