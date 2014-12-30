// DllEvent.cpp: implementation of the CDllEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "DllEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDllEvent::CDllEvent()
{

}

CDllEvent::~CDllEvent()
{

}

/*
typedef struct _LOAD_DLL_DEBUG_INFO { 
    HANDLE hFile; 
    LPVOID lpBaseOfDll; 
    DWORD  dwDebugInfoFileOffset; 
    DWORD  nDebugInfoSize; 
    LPVOID lpImageName; 
    WORD fUnicode; 
} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;
*/
DWORD CDllEvent::OnLoad(const CBaseEvent *pEvent)
{
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    
    m_pUI->ShowInfo("Dll Loaded: ");
    GetImageName(pEvent);
    
    return dwContinueStatus;
}

DWORD CDllEvent::OnUnload(const CBaseEvent *pEvent)
{   
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 

    UNLOAD_DLL_DEBUG_INFO unloadInfo = (UNLOAD_DLL_DEBUG_INFO)pEvent->m_debugEvent.u.UnloadDll;
    sprintf(g_szBuf, "Dll Unloaded: %p\r\n", unloadInfo.lpBaseOfDll);
    m_pUI->ShowInfo(g_szBuf);

    return dwContinueStatus;
}

DWORD CDllEvent::OnOutputString(const CBaseEvent *pEvent)
{
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    BOOL bRet;

    OUTPUT_DEBUG_STRING_INFO outputInfo = (OUTPUT_DEBUG_STRING_INFO)pEvent->m_debugEvent.u.DebugString;
    DWORD nLen = outputInfo.nDebugStringLength;
    char *pstrBuf = NULL;
    char *pstrBufA = NULL;

    pstrBuf = new char[nLen];
    pstrBufA = new char[nLen + 32];
    if (NULL == pstrBuf
        || NULL == pstrBufA)
    {
        CUI::ShowErrorMessage();
        return dwContinueStatus;
    }

    bRet = ReadProcessMemory(pEvent->m_hProcess,
                            outputInfo.lpDebugStringData,     
                            pstrBuf,
                            nLen,
                            NULL);
    if (!bRet)
    {
        CUI::ShowErrorMessage();
        return dwContinueStatus;
    }

    if (outputInfo.fUnicode)
    {
        sprintf(pstrBufA, "DebugString: %S\r\n", pstrBuf);
        m_pUI->ShowInfo(pstrBufA);
    }
    else
    {
        m_pUI->ShowInfo("DebugString: ");
        m_pUI->ShowInfo(pstrBuf);
        m_pUI->ShowInfo("\r\n");
    }

    if (pstrBuf != NULL)
    {
        delete[] pstrBuf;
        pstrBuf = NULL;
    }

    if (pstrBufA != NULL)
    {
        delete[] pstrBufA;
        pstrBufA = NULL;
    }

    return dwContinueStatus;
}

/************************************************************************/
/* 
Function : try to get dll name                                          
Return   : TRUE if success, FALSE otherwise                                                             */
/************************************************************************/
BOOL CDllEvent::GetImageName(const CBaseEvent *pEvent)
{
    assert(pEvent != NULL);

    HANDLE hProcess = pEvent->m_hProcess;
    char szBuf[MAX_PATH * 2];
    char szBufA[MAX_PATH];
    DWORD ptrImageName;
    BOOL bRet;
    LOAD_DLL_DEBUG_INFO loadDllInfo = (LOAD_DLL_DEBUG_INFO)pEvent->m_debugEvent.u.LoadDll;
    if (NULL == hProcess
        || NULL == loadDllInfo.lpImageName)
    {
        return FALSE;
    }

    //Debuggers must be prepared to handle the case where lpImageName is NULL
    //or *lpImageName (in the address space of the process being debugged) is NULL.
    bRet = ReadProcessMemory(hProcess,
                             loadDllInfo.lpImageName,
                             &ptrImageName,
                             sizeof(DWORD),
                             NULL);

    if (!bRet)
    {
        CUI::ShowErrorMessage();
        return FALSE;
    }

    bRet = ReadProcessMemory(hProcess,
                            (LPVOID)ptrImageName,
                            szBuf,
                            MAX_PATH * 2,
                            NULL);

    if (!bRet)
    {
        sprintf(szBufA, "%p ", loadDllInfo.lpBaseOfDll);
        m_pUI->ShowInfo(szBufA);
        CUI::ShowErrorMessage();
        return FALSE;
    }

    if (loadDllInfo.fUnicode)
    {
        _snprintf(szBufA, MAX_PATH, "%p %S\r\n", loadDllInfo.lpBaseOfDll, szBuf);
        m_pUI->ShowInfo(szBufA);
    }
    else
    {
        sprintf(szBufA, "%p ", loadDllInfo.lpBaseOfDll);
        m_pUI->ShowInfo(szBufA);
        m_pUI->ShowInfo(szBuf);
        m_pUI->ShowInfo("\r\n");
    }

    return TRUE;
}
