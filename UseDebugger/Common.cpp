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
