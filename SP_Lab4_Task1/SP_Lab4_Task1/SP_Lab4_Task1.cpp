// SP_Lab4_Task1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <strsafe.h>

HRESULT Execute7zip(TCHAR* cmd);

HANDLE stdin_pipe_write = NULL;
HANDLE stdin_pipe_read = NULL;
HANDLE stdout_pipe_write = NULL;
HANDLE stdout_pipe_read = NULL;

int _tmain(UINT argc, TCHAR** argv)
{
	SECURITY_ATTRIBUTES saAttr;
	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&stdout_pipe_read, &stdout_pipe_write, &saAttr, 0))
	{
		_tprintf(_T("Error create pipe.\n"));
		return 0;
	}
	if (!SetHandleInformation(stdout_pipe_read, HANDLE_FLAG_INHERIT, 0))
	{
		_tprintf(_T("Error protect stdout pipe.\n"));
		return 0;
	}

	if (argc != 3)
	{
		_tprintf(_T("Pass key -x or -a and filename.\n"));
	}

	
	TCHAR* cmd = (TCHAR*)LocalAlloc(LPTR, MAX_PATH * sizeof(TCHAR));
	
	if (_tcscmp(_T("-x"), argv[1]) == 0)
	{
		_swprintf(cmd, _T("7z.exe x -y -- %s"), argv[2]);
		// extraction is asked
		Execute7zip(cmd);
	}
	else if (_tcscmp(_T("-a"), argv[1]) == 0)
	{
		_swprintf(cmd, _T("7z.exe a -y -- %s.zip %s"), argv[2], argv[2]);
		// compression is asked
		Execute7zip(cmd);
	}
	else
	{
		_tprintf(_T("Unknown key."));
	}

	_tsystem(_T("pause"));
    return 0;
}

void ErrorExit(PTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	_tprintf((LPCTSTR)lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

void ReadFromPipe(void)
{
	DWORD dwRead = 0;
	CHAR buffer[1024] = { 0 };
	
	ReadFile(stdout_pipe_read, buffer, 4096, &dwRead, NULL);
	if (dwRead != 0)
	{
		_tprintf(_T("Error had happend.\n"));
		WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buffer, dwRead, NULL, NULL);
	}
	else
	{
		_tprintf(_T("Successful operation.\n"));
	}
}

HRESULT Execute7zip(TCHAR* cmd)
{
	PROCESS_INFORMATION process_information = { 0 };
	STARTUPINFO startup_info = { 0 };
	HRESULT result_status = S_OK;

	startup_info.cb = sizeof(STARTUPINFO);
	startup_info.hStdError = stdout_pipe_write;
	startup_info.dwFlags |= STARTF_USESTDHANDLES;

	if (
		!CreateProcess(NULL,
		cmd, 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&startup_info,
		&process_information)  // receives PROCESS_INFORMATION
		)
	{
		result_status = ERROR;
		ErrorExit(_T("Create process error."));
	}
	else
	{
		ReadFromPipe();

		CloseHandle(process_information.hProcess);
		CloseHandle(process_information.hThread);
	}

	return result_status;
}
