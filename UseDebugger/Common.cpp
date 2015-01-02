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

/************************************************************************/
/* 
Function : Try to load the specified file into memory,
Params   : pszFileName used to indicate the filepath
           ppFileBuf used to receive the buffer pointer
                Do not forget to free the buf if no longer used
Return   : TRUE if success, FALSE otherwise
*/
/************************************************************************/
BOOL
LoadFile(char *pszFileName, char **ppFileBuf, long *pnFileSize)
{
    assert(pszFileName != NULL);
    assert(ppFileBuf != NULL);
    *ppFileBuf = NULL;

    FILE *fp    = NULL;
    char *pBuf  = NULL;
    int  nRet   = 0;
    BOOL bRet   = TRUE;
    long nFileSize = 0;

    fp = fopen(pszFileName, "rb");
    if (NULL == fp)
    {
        bRet = FALSE;
        goto END;
    }

    //get file size
    nRet = fseek(fp, 0, SEEK_END);
    if (nRet)
    {   
        bRet = FALSE;
        goto END;
    }

    nFileSize = ftell(fp);
    if (-1L == nFileSize)
    {
        bRet = FALSE;
        goto END;
    }

    //rollback
    nRet = fseek(fp, 0, SEEK_SET);
    if (nRet)
    {
        bRet = FALSE;
        goto END;
    }

    //alloc mem and load file into
    pBuf = (char *)malloc(nFileSize);
    if (NULL == pBuf)
    {
        bRet = FALSE;
        goto END;
    }

    fread(pBuf, sizeof(char), nFileSize, fp);
    if(ferror(fp))
    {
        bRet = FALSE;
        goto END;
    }

    *ppFileBuf  = pBuf;
    *pnFileSize = nFileSize;

END:
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
    
    return bRet;
}
