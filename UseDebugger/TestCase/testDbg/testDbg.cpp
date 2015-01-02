// testDbg.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <windows.h>


DWORD WINAPI ThreadProc( LPVOID lpParameter )
{
    return 0;
}

int main(int argc, char* argv[])
{
	OutputDebugString( "123");
    MessageBox(NULL, "main", NULL, NULL);
    DWORD dwThreadId = 0;
    HANDLE hThread = CreateThread(NULL, NULL, ThreadProc, NULL, NULL, &dwThreadId);
    
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi = {0};
    CreateProcess(NULL,  "notepad.exe", NULL,  NULL,  FALSE,   0,  NULL, NULL,  &si,  &pi);
	WaitForSingleObject(hThread, INFINITE);
	char* p = NULL;
	*p = 123;
	printf("Hello World!\n");
	return 0;
}

