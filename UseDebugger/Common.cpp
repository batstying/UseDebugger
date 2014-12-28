#include "Common.h"

char g_szBuf[MAXBUF];

void
SafeClose(HANDLE handle)
{
    if (handle != NULL)
    {
        CloseHandle(handle);
        handle = NULL;
    }
}

void 
ShowErrorMessage()
{
    LPVOID lpMsgBuf;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR) &lpMsgBuf,
        0,
        NULL 
        );
    
    printf((const char *)lpMsgBuf);
    printf("\r\n");
    LocalFree(lpMsgBuf);
}