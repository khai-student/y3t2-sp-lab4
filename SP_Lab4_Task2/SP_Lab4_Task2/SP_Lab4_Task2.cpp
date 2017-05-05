// SP_Lab4_Task2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define THREADCOUNT 10
#define DATA_LENGTH 10

DWORD tls_data_pointer = 0;
DWORD tls_gcd_cells[DATA_LENGTH] = { 0 };
DWORD tls_summary_pointer = 0;

VOID ErrorExit(TCHAR*);
VOID CalculateGcd(VOID);
VOID CalculateSummary(VOID);
DWORD WINAPI ThreadFunc(VOID*);

INT _tmain(UINT argc, TCHAR** argv)
{
	// set of rand seed
	srand((unsigned)time(NULL));

	// Parse cmd line args
	if (argc != 3 || _tcscmp(_T("-n"), argv[1]))
	{
		ErrorExit(_T("Wrong cmd-line args. Example: prog.exe -n 10"));
	}
	DWORD thread_count = _tstoi(argv[2]);
	if (!thread_count)
	{
		ErrorExit(_T("Wrong number of threads."));
	}

	DWORD return_value = 0;
	HANDLE* threads = (HANDLE*)LocalAlloc(LPTR, thread_count * sizeof(HANDLE));
	int i = 0;

	// Allocate a TLS cells. 

	if ((tls_data_pointer = TlsAlloc()) == TLS_OUT_OF_INDEXES)
	{
		ErrorExit(_T("TlsAlloc failed"));
	}
	if ((tls_summary_pointer = TlsAlloc()) == TLS_OUT_OF_INDEXES)
	{
		ErrorExit(_T("TlsAlloc failed"));
	}
	for (i = 0; i < DATA_LENGTH; ++i)
	{
		if ((tls_gcd_cells[i] = TlsAlloc()) == TLS_OUT_OF_INDEXES)
		{
			ErrorExit(_T("TlsAlloc failed"));
		}
	}

	// Create multiple threads. 

	for (i = 0; i < THREADCOUNT; ++i)
	{
		// generate array
		DWORD* data = (DWORD*)LocalAlloc(LPTR, DATA_LENGTH * sizeof(DWORD));
		// fill array
		for (DWORD value_index = 0; value_index < DATA_LENGTH; value_index++)
		{
			data[value_index] = (rand() % 91) + 10;
		}
		// launch thread
		threads[i] = CreateThread(NULL, // default security attributes 
			0,                           // use default stack size 
			(LPTHREAD_START_ROUTINE)ThreadFunc, // thread function 
			(VOID*)data,
			0,                       // use default creation flags 
			&return_value);              // returns thread identifier 
		
		// Check the return value for success. 
		if (threads[i] == NULL)
		{
			ErrorExit(_T("CreateThread error\n"));
		}
	}

	for (i = 0; i < THREADCOUNT; ++i)
	{
		WaitForSingleObject(threads[i], INFINITE);
	}

	// free tls memory
	TlsFree(tls_data_pointer);
	TlsFree(tls_summary_pointer);
	for (i = 0; i < DATA_LENGTH; ++i)
	{
		TlsFree(tls_gcd_cells[i]);
	}

	LocalFree(threads);
	return 0;
}

VOID ErrorExit(TCHAR* msg)
{
	_tprintf(_T("%s\n"), msg);
	ExitProcess(0);
}

DWORD WINAPI ThreadFunc(VOID* data)
{
	if (!TlsSetValue(tls_data_pointer, data))
	{
		ErrorExit(_T("TlsSetValue error"));
	}

	CalculateGcd();

	for (DWORD i = 0, val; i < DATA_LENGTH; i++)
	{
		val = (DWORD)TlsGetValue(tls_gcd_cells[i]);
	}

	CalculateSummary();

	_tprintf(_T("Thread: %d\tSummary: %d\n"), GetCurrentThreadId(), (DWORD)TlsGetValue(tls_summary_pointer));
	// Release the dynamic memory before the thread returns. 
	LocalFree((HLOCAL)data);
	return 0;
}

VOID CalculateGcd(VOID)
{
	DWORD number_1 = 0, number_2 = 0;
	for (DWORD index_1 = 0, index_2 = index_1 + 1; index_1 < DATA_LENGTH; ++index_1, ++index_2)
	{
		if (index_2 == DATA_LENGTH)
		{
			index_2 = 0;
		}
		number_1 = *((DWORD*)TlsGetValue(tls_data_pointer) + index_1);
		number_2 = *((DWORD*)TlsGetValue(tls_data_pointer) + index_2);
		// calculating gcd
		while (number_1 && number_2)
		{
			number_1 %= number_2;
			number_1 ^= number_2 ^= number_1 ^= number_2;
		}

		TlsSetValue(tls_gcd_cells[index_1], (VOID*)(number_1 + number_2));
	}
}

VOID CalculateSummary(VOID)
{
	DWORD summary = 0;

	for (DWORD i = 0; i < DATA_LENGTH; ++i)
	{
		summary += (DWORD)TlsGetValue(tls_gcd_cells[i]);
	}

	TlsSetValue(tls_summary_pointer, (VOID*)summary);
}