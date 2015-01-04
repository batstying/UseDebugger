// DllEvent.cpp: implementation of the CDllEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "DllEvent.h"
#include "UseDebugger.h"
#include "PE.H"

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
DWORD CDllEvent::OnLoad(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    tagModule module = {0};
    
    m_pUI->ShowInfo("Dll Loaded: ");
    GetModuleInfo(pEvent, &module);

    //retrieve image size, OEP
    if (module.hFile != NULL)
    {
        ::SetImageBuf(module.hFile);
    }
    else 
    {
        //with the help of PE.cpp
        ::LoadFile(module.szPath);
    }
    ::GetModuleInfo(&module);

    //update modules
    m_mapBase_Module[module.modBaseAddr] = module;

    //for every loaded dll, consider whether necessary to trace the instruction
    if (pEvent->m_bTrace)
    {
        int argv[] = {0, 3, 0x0C, 0x0E};
        sprintf(g_szBuf, "bm %p a %d", 
                          module.dwBaseOfCode,
                          module.dwSizeOfCode
                          );
        ((CUseDebugger *)pEvent)->DoBM(4, argv, g_szBuf, TRUE);
        ((CUseDebugger *)pEvent)->m_bTalk = FALSE;       //no need to interact with the user
    }
    
    return dwContinueStatus;
}

DWORD CDllEvent::OnUnload(CBaseEvent *pEvent)
{   
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 

    UNLOAD_DLL_DEBUG_INFO unloadInfo = (UNLOAD_DLL_DEBUG_INFO)pEvent->m_debugEvent.u.UnloadDll;
    sprintf(g_szBuf, "Dll Unloaded: %p\r\n", unloadInfo.lpBaseOfDll);
    m_pUI->ShowInfo(g_szBuf);

    //remove MemBP used for trace
    tagModule *pModule = &m_mapBase_Module[(DWORD)unloadInfo.lpBaseOfDll];
    if (pEvent->m_bTrace)
    {
        ((CUseDebugger *)pEvent)->RemoveTrace(pModule);
        ((CUseDebugger *)pEvent)->m_bTalk = FALSE;
    }

    //update modules
    m_mapBase_Module.erase((DWORD)unloadInfo.lpBaseOfDll);

    return dwContinueStatus;
}

DWORD CDllEvent::OnOutputString(CBaseEvent *pEvent)
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

    bRet = ((CUseDebugger *)pEvent)->ReadBuf(pEvent,
                            pEvent->m_hProcess, 
                            outputInfo.lpDebugStringData,     
                            pstrBuf,
                            nLen);
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
Function : try to get dll info  
Params   : pModule used to receive module info                                        
Return   : TRUE if success, FALSE otherwise                                                             */
/************************************************************************/
BOOL CDllEvent::GetModuleInfo(CBaseEvent *pEvent, tagModule *pModule)
{
    assert(pEvent != NULL);
    assert(pModule != NULL);

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
    bRet = ((CUseDebugger *)pEvent)->ReadBuf(pEvent,
                                        hProcess,
                                        loadDllInfo.lpImageName,
                                        &ptrImageName,
                                        sizeof(DWORD)
                                        );

    if (!bRet)
    {
        return FALSE;
    }

    bRet = ((CUseDebugger *)pEvent)->ReadBuf(pEvent,
                                          hProcess,
                                          (LPVOID)ptrImageName,
                                          szBuf,
                                          MAX_PATH * 2
                                          );

    if (!bRet)
    {
        sprintf(szBufA, "%p ", loadDllInfo.lpBaseOfDll);
        m_pUI->ShowInfo(szBufA);
        CUI::ShowErrorMessage();
        return FALSE;
    }

    if (loadDllInfo.fUnicode)
    {
        //module info
        _snprintf(pModule->szPath, MAX_PATH, "%S", szBuf);

        _snprintf(szBufA, MAX_PATH, "%p %S\r\n", loadDllInfo.lpBaseOfDll, szBuf);
        m_pUI->ShowInfo(szBufA);
    }
    else
    {
        //module info
        _snprintf(pModule->szPath, MAX_PATH, "%s", szBuf);

        sprintf(szBufA, "%p ", loadDllInfo.lpBaseOfDll);
        m_pUI->ShowInfo(szBufA);
        m_pUI->ShowInfo(szBuf);
        m_pUI->ShowInfo("\r\n");
    }

    //module info
    pModule->modBaseAddr = (DWORD)loadDllInfo.lpBaseOfDll;
    pModule->hFile       = loadDllInfo.hFile;

    return TRUE;
}

/************************************************************************/
/* 
Function : list all the modules within the debuggee                                                                     */
/************************************************************************/
BOOL 
CDllEvent::DoListModule(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/)
{
    sprintf(g_szBuf, "-------------Module List-------------\r\n"
                     "Range\t\t\tOEP\t\tPath\r\n");

    tagModule *pModule;
    map<DWORD, tagModule>::iterator it;
    for (it = m_mapBase_Module.begin();
          it != m_mapBase_Module.end();
          it++)
    {
        pModule = &it->second;
        _snprintf(g_szBuf, MAXBUF, "%s%p-%p\t%p\t%s\r\n",
                                    g_szBuf,
                                    pModule->modBaseAddr,
                                    pModule->modBaseAddr + pModule->modBaseSize,
                                    pModule->dwOEP,
                                    pModule->szPath);
    }

    CUI::ShowInfo(g_szBuf);
    return TRUE;
}

/************************************************************************/
/* 
Function : set MemBP for the loaded module, used for tracing instruction  */
/************************************************************************/
BOOL 
CDllEvent::DoTrace(CBaseEvent *pEvent)
{
    int argv[] = {0, 3, 0x0C, 0x0E};
    tagModule *pModule;
    
    map<DWORD, tagModule>::iterator it;
    for (it = m_mapBase_Module.begin();
          it != m_mapBase_Module.end();
          it++)
    {
        pModule = &it->second;
        sprintf(g_szBuf, "bm %p a %d", 
                         pModule->dwBaseOfCode, 
                         pModule->dwSizeOfCode);
        ((CUseDebugger *)pEvent)->DoBM(4, argv, g_szBuf, TRUE);
    }
    return TRUE;
}