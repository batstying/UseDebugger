#ifndef COMMON_H_DBFC83AC_0FC7_4a51_A52F_6B60B2927A8C
#define COMMON_H_DBFC83AC_0FC7_4a51_A52F_6B60B2927A8C

#pragma warning(disable: 4786)
#include <windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <map>
#include <list>
#include <vector>
#include <algorithm>
using namespace std;

#define MAXBUF  (1024 * 100)
extern char g_szBuf[MAXBUF];

//////////////////////////////////////////////////////////////////////////
//structures for dll 
typedef struct _tagModule
{
    DWORD   modBaseAddr;
    DWORD   modBaseSize; 
    DWORD   dwOEP;
    HANDLE  hFile;          //from LOAD_DLL_DEBUG_INFO
    DWORD   dwBaseOfCode;   
    DWORD   dwSizeOfCode;
    char   szPath[MAX_PATH]; 
}tagModule;

//prototype
extern "C"
void
__stdcall
Decode2AsmOpcode(IN PBYTE pCodeEntry,   // 需要解析指令地址
                 OUT char* strAsmCode,        // 得到反汇编指令信息
                 OUT char* strOpcode,         // 解析机器码信息
                 OUT UINT* pnCodeSize,        // 解析指令长度
                 IN UINT nAddress);

void
SafeClose(HANDLE handle);

/************************************************************************/
/* 
Function : Try to load the specified file into memory,
Params   : pszFileName used to indicate the filepath
           ppFileBuf used to receive the buffer pointer
                Do not forget to free the buf if no longer used

           pnFileSize used to receive the filesize
Return   : TRUE if success, FALSE otherwise
*/
/************************************************************************/
BOOL
LoadFile(char *pszFileName, char **ppFileBuf, long *pnFileSize);


#endif