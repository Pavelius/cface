#include "cface/crt.h"
#include "cface/terminal.h"

#define WINUSERAPI extern "C"
#define WINAPI __stdcall

typedef long LRESULT;
typedef long LONG;
typedef void* HWND;
typedef unsigned UINT;
typedef unsigned DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#define	STILL_ACTIVE 259
#define INFINITE 0xFFFFFFFF

#define HANDLE_FLAG_INHERIT 0x00000001
#define HANDLE_FLAG_PROTECT_FROM_CLOSE 0x00000002

#define STARTF_FORCEONFEEDBACK 0x00000040
#define STARTF_FORCEOFFFEEDBACK 0x00000080
#define STARTF_PREVENTPINNING 0x00002000
#define STARTF_RUNFULLSCREEN 0x00000020
#define STARTF_TITLEISAPPID 0x00001000
#define STARTF_TITLEISLINKNAME 0x00000800
#define STARTF_UNTRUSTEDSOURCE 0x00008000
#define STARTF_USESTDHANDLES 0x00000100

struct SECURITY_ATTRIBUTES {
	unsigned				nLength;
	void*					lpSecurityDescriptor;
	int						bInheritHandle;
};
struct STARTUPINFO {
	unsigned				cb;
	const char*				lpReserved;
	const char*				lpDesktop;
	const char*				lpTitle;
	unsigned				dwX;
	unsigned				dwY;
	unsigned				dwXSize;
	unsigned				dwYSize;
	unsigned				dwXCountChars;
	unsigned				dwYCountChars;
	unsigned				dwFillAttribute;
	unsigned				dwFlags;
	unsigned				wShowWindow;
	unsigned				cbReserved2;
	unsigned char*			lpReserved2;
	void*					hStdInput;
	void*					hStdOutput;
	void*					hStdError;
};
struct PROCESS_INFORMATION {
	void*					hProcess;
	void*					hThread;
	unsigned				dwProcessId;
	unsigned				dwThreadId;
};

WINUSERAPI int WINAPI		CloseHandle(void* hObject);
WINUSERAPI int WINAPI		CreatePipe(void** hReadPipe, void** hWritePipe, SECURITY_ATTRIBUTES* lpPipeAttributes, unsigned nSize);
WINUSERAPI int WINAPI		CreateProcessA(const char* lpApplicationName, const char* lpCommandLine, SECURITY_ATTRIBUTES* lpProcessAttributes, SECURITY_ATTRIBUTES* lpThreadAttributes, int bInheritHandles, unsigned dwCreationFlags, void* lpEnvironment, const char* lpCurrentDirectory, STARTUPINFO* lpStartupInfo, PROCESS_INFORMATION* lpProcessInformation);
WINUSERAPI int WINAPI		GetExitCodeProcess(void* hProcess, unsigned* lpExitCode);
WINUSERAPI int WINAPI		PeekNamedPipe(void* hNamedPipe, void* lpBuffer, unsigned nBufferSize, unsigned* lpBytesRead, unsigned* lpTotalBytesAvail, unsigned* lpBytesLeftThisMessage);
WINUSERAPI int WINAPI		ReadFile(void* hFile, void* lpBuffer, unsigned nNumberOfBytesToRead, unsigned* lpNumberOfBytesRead, void* lpOverlapped);
WINUSERAPI int WINAPI		SetHandleInformation(void* hObject, unsigned dwMask, unsigned dwFlags);

terminal::terminal() :child_out_rd(0), child_out_wr(0),
child_in_rd(0), child_in_wr(0),
child_proc(0), child_thrd(0)
{
	SECURITY_ATTRIBUTES sa = {0};
	// Set the bInheritHandle flag so pipe handles are inherited.
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = 1;
	// Create a pipe for the child process's STDOUT.
	if(!CreatePipe(&child_out_rd, &child_out_wr, &sa, 0))
		return;
	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if(!SetHandleInformation(child_out_rd, HANDLE_FLAG_INHERIT, 0))
		return;
	// Create a pipe for the child process's STDIN.
	if(!CreatePipe(&child_in_rd, &child_in_wr, &sa, 0))
		return;
	// Ensure the write handle to the pipe for STDIN is not inherited.
	if(!SetHandleInformation(child_in_wr, HANDLE_FLAG_INHERIT, 0))
		return;
}

static void close_handle(void*& e)
{
	if(e)
	{
		CloseHandle(e);
		e = 0;
	}
}

terminal::~terminal()
{
	close_handle(child_out_rd);
	close_handle(child_out_wr);
	close_handle(child_in_rd);
	close_handle(child_in_wr);
}

terminal::operator bool() const
{
	return child_in_rd && child_out_rd
		&& child_in_wr && child_out_wr;
}

bool terminal::launch(const char* url)
{
	PROCESS_INFORMATION	pi = {0};
	STARTUPINFO			si = {0};
	//
	status = 0;
	si.cb = sizeof(si);
	si.hStdError = child_out_wr;
	si.hStdOutput = child_out_wr;
	si.hStdInput = child_in_rd;
	si.dwFlags |= STARTF_USESTDHANDLES;
	// Create the child process.
	if(!CreateProcessA(url,
		0,			// command line
		0,          // process security attributes
		0,          // primary thread security attributes
		1,          // handles are inherited
		0,          // creation flags
		0,          // use parent's environment
		0,          // use parent's current directory
		&si,   		// STARTUPINFO pointer
		&pi))
		return false;  // receives PROCE
	child_proc = pi.hProcess;
	child_thrd = pi.hThread;
	status = STILL_ACTIVE;
	return true;
}

int terminal::read(char* buff, int count)
{
	if(!child_out_rd)
		return 0;
	unsigned bytesAvailable = 0;
	if(!PeekNamedPipe(child_out_rd, 0, 0, 0, &bytesAvailable, 0))
		return 0;
	if(bytesAvailable)
	{
		if(!ReadFile(child_out_rd, buff, imin(count, (int)bytesAvailable), &bytesAvailable, 0))
			return 0;
		return bytesAvailable;
	}
	return 0;
}

bool terminal::isactive()
{
	if(!child_proc)
		return false;
	GetExitCodeProcess(child_proc, &status);
	if(status==STILL_ACTIVE)
		return true;
	child_proc = 0;
	child_thrd = 0;
	close_handle(child_out_rd);
	close_handle(child_out_wr);
	close_handle(child_in_rd);
	close_handle(child_in_wr);
	return false;
}