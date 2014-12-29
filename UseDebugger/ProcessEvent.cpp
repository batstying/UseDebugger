// ProcessEvent.cpp: implementation of the CProcessEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "ProcessEvent.h"
#include "UseDebugger.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcessEvent::CProcessEvent()
{

}

CProcessEvent::~CProcessEvent()
{

}

/*
typedef struct _CREATE_PROCESS_DEBUG_INFO {
    HANDLE hFile; 
    HANDLE hProcess; 
    HANDLE hThread; 
    LPVOID lpBaseOfImage; 
    DWORD dwDebugInfoFileOffset; 
    DWORD nDebugInfoSize; 
    LPVOID lpThreadLocalBase; 
    LPTHREAD_START_ROUTINE lpStartAddress; 
    LPVOID lpImageName; 
    WORD fUnicode; 
} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;
*/
DWORD CProcessEvent::OnCreateProcess(CBaseEvent *pEvent)
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    CREATE_PROCESS_DEBUG_INFO processInfo = (CREATE_PROCESS_DEBUG_INFO)pEvent->m_debugEvent.u.CreateProcessInfo;
    _snprintf(g_szBuf, MAXBUF, "----------------Process Created-------\r\n"
                               "OEP: %p ImageBase: %p\r\n\r\n",
                               processInfo.lpStartAddress,
                               processInfo.lpBaseOfImage);
    pEvent->m_pUI->ShowInfo(g_szBuf);

    //set BP at OEP
    pEvent->m_dwOEP = (DWORD)processInfo.lpStartAddress;

    strcpy(g_szBuf, "bp");
    sprintf(&g_szBuf[3], "%p", processInfo.lpStartAddress);
    int argv[] = {0, 3};
    ((CUseDebugger *)pEvent)->DoBP(2, argv, g_szBuf);

    return dwContinueStatus;
}

DWORD CProcessEvent::OnCreateThread(const CBaseEvent *pEvent)
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    return dwContinueStatus;
}

DWORD CProcessEvent::OnExitThread(const CBaseEvent *pEvent)
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    return dwContinueStatus;
}

DWORD CProcessEvent::OnExitProcess(const CBaseEvent *pEvent)
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    return dwContinueStatus;
}
