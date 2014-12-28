// UseDebugger.cpp: implementation of the CUseDebugger class.
//
//////////////////////////////////////////////////////////////////////
#include "UseDebugger.h"

//////////////////////////////////////////////////////////////////////////
// #define EXCEPTION_DEBUG_EVENT       1
// #define CREATE_THREAD_DEBUG_EVENT   2
// #define CREATE_PROCESS_DEBUG_EVENT  3
// #define EXIT_THREAD_DEBUG_EVENT     4
// #define EXIT_PROCESS_DEBUG_EVENT    5
// #define LOAD_DLL_DEBUG_EVENT        6
// #define UNLOAD_DLL_DEBUG_EVENT      7
// #define OUTPUT_DEBUG_STRING_EVENT   8
// #define RIP_EVENT                   9
//#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
//#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
//#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP

//成员函数指针, 用于事件的分发，
typedef DWORD (CUseDebugger::*PFNDispatchEvent)(void);
static map<DWORD, PFNDispatchEvent> gs_mapEventID_PFN;

typedef void (CUseDebugger::*PFNDispatchInput)(int argc, int pargv[], const char *pszBuf);
static map<char *, PFNDispatchInput> gs_mapInput_PFN;

/************************************************************************/
/* 
Function : to make the rule: which command should be processed by which func
*/
/************************************************************************/
void
CUseDebugger::DispatchCommand()
{
#define DISPATCHEVENT(ID, pfn)  gs_mapEventID_PFN[ID] = pfn;
    DISPATCHEVENT(EXCEPTION_DEBUG_EVENT,     CUseDebugger::OnExceptDispatch)
    DISPATCHEVENT(CREATE_THREAD_DEBUG_EVENT, CUseDebugger::OnCreateThread)
    DISPATCHEVENT(CREATE_PROCESS_DEBUG_EVENT,CUseDebugger::OnCreateProcess)
    DISPATCHEVENT(EXIT_THREAD_DEBUG_EVENT,   CUseDebugger::OnExitThread)
    DISPATCHEVENT(EXIT_PROCESS_DEBUG_EVENT,  CUseDebugger::OnExitProcess)
    DISPATCHEVENT(LOAD_DLL_DEBUG_EVENT,      CUseDebugger::OnLoadDLL)
    DISPATCHEVENT(UNLOAD_DLL_DEBUG_EVENT,    CUseDebugger::OnUnLoadDLL)
    DISPATCHEVENT(OUTPUT_DEBUG_STRING_EVENT, CUseDebugger::OnOutputDebugString)
    DISPATCHEVENT(EXCEPTION_ACCESS_VIOLATION, CUseDebugger::OnAccessViolation)
    DISPATCHEVENT(EXCEPTION_BREAKPOINT,       CUseDebugger::OnBreakPoint)
    DISPATCHEVENT(EXCEPTION_SINGLE_STEP,      CUseDebugger::OnSingleStep)

#define DISPATCHINPUT(str, pfn)  gs_mapInput_PFN[str] = pfn;
    //DISPATCHINPUT("bp",)

    
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUseDebugger::CUseDebugger()
{
    m_pDllEvent = NULL;
    m_pDllEvent = new CDllEvent;
    assert(m_pDllEvent != NULL);

    m_pProcessEvent = NULL;
    m_pProcessEvent = new CProcessEvent;
    assert(m_pProcessEvent != NULL);

    this->DispatchCommand();
}

CUseDebugger::~CUseDebugger()
{
    //nothing 
}

/************************************************************************/
/*  
Function : To make sure that only one instance for this time           */
/************************************************************************/
CUseDebugger *
CUseDebugger::CreateSystem(void)
{
    static CUseDebugger *pobj = new CUseDebugger;
    return pobj;
}

void
CUseDebugger::DestorySystem(void)
{   
    delete this;
}

void
CUseDebugger::Run(void)
{   
    BOOL bRet = TRUE;
    char ch;
    while (true)
    {
        m_pUI->ShowMainMenu();
        m_pUI->GetCH(&ch);
       
        if (ch == '1')
        {
            bRet = this->DebugNewProcess();
        }
        else if (ch == '2')
        {
            //TBD   
        }
        else if (ch == '0')
        {
            break;
        }
    }
}

BOOL
CUseDebugger::DebugNewProcess()
{
    //////////////////////////////////////////////////////////////////////////
    //Select file u want to debug
    BOOL bRet = TRUE;
    char szFilePath[MAX_PATH];

    bRet = m_pUI->SelectFile(szFilePath);
    if (!bRet)
    {
        return bRet;
    }
    
    //////////////////////////////////////////////////////////////////////////
    PROCESS_INFORMATION pi  = {0};
    STARTUPINFO si			= {0};
    si.cb = sizeof(STARTUPINFO);
    
    bRet = ::CreateProcess(NULL, 
                    szFilePath, 
                    NULL,
                    NULL, 
                    FALSE, 
                    DEBUG_ONLY_THIS_PROCESS,
                    NULL, 
                    NULL, 
                    &si,
                    &pi);
    if(!bRet)
    {
        ::ShowErrorMessage();
        return FALSE;
	}

    this->DebugProcess();

    return TRUE;
}

BOOL
CUseDebugger::DebugAttachedProcess()
{
    //TBD
    return TRUE;
}

/************************************************************************/
/* 
Function : the main part for debugging process, event dispatch                                                                     */
/************************************************************************/
BOOL
CUseDebugger::DebugProcess()
{
    //to fix error C2065: 'OpenThread' : undeclared identifier
    typedef HANDLE (WINAPI *OPENTHREAD)(DWORD, BOOL, DWORD);
    OPENTHREAD pfnOpenThread = (OPENTHREAD)GetProcAddress(GetModuleHandle("Kernel32"), "OpenThread");
    assert(pfnOpenThread != NULL);

    //about input
    int argc;
    int pargv[MAXBYTE];

    map<DWORD, PFNDispatchEvent>::iterator it;
    map<DWORD, PFNDispatchEvent>::iterator itend = gs_mapEventID_PFN.end();
    PFNDispatchEvent pfnEvent = NULL;

    BOOL bRet       = TRUE;                      
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    while (TRUE)
    {         
        bRet = ::WaitForDebugEvent(&m_debugEvent, INFINITE); 
        if (!bRet)
        {
            ::ShowErrorMessage();
            return FALSE;
        }

        dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
        m_bTalk = FALSE;

        //prefetch these info: hProcess, hThread, context
        m_hProcess = ::OpenProcess(
                            PROCESS_ALL_ACCESS, //PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION ,
                            FALSE, 
                            m_debugEvent.dwProcessId
                            );
        if (NULL == m_hProcess)
        {
            ::ShowErrorMessage();
            return FALSE;
        }
            
        m_hThread = pfnOpenThread(
                    THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, 
                    FALSE,
                    m_debugEvent.dwThreadId
                    );
        if (NULL == m_hThread)
        {
            ::ShowErrorMessage();
            return FALSE;
        }
         
        m_Context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
        bRet = ::GetThreadContext(m_hThread, &m_Context);
        if (!bRet)
        {
            ::ShowErrorMessage();
            return FALSE;
        }
        
        //事件分发
        it = gs_mapEventID_PFN.find(m_debugEvent.dwDebugEventCode);
        if (it != itend)
        {
            pfnEvent = (*it).second;
            dwContinueStatus = (this->*pfnEvent)();   
        }

        //interact with the user
        if (m_bTalk)
        {
            this->CBaseEvent::ShowRegs();

            m_pUI->GetInput(&argc, pargv, g_szBuf, MAXBUF);

            

            m_bTalk = FALSE;
        }

        //restore context, and close handles      
        bRet = ::SetThreadContext(m_hThread, &m_Context);
        if (!bRet)
        {
            ::ShowErrorMessage();
            return FALSE;
        }
            
        ::SafeClose(m_hThread);
        ::SafeClose(m_hProcess);            
                
        ::ContinueDebugEvent(
                m_debugEvent.dwProcessId, 
                m_debugEvent.dwThreadId, 
                dwContinueStatus); 
        
    } 

    return TRUE;
}

DWORD
CUseDebugger::OnExceptDispatch()
{
    map<DWORD, PFNDispatchEvent>::iterator it;
    map<DWORD, PFNDispatchEvent>::iterator itend = gs_mapEventID_PFN.end();
    PFNDispatchEvent pfnEvent = NULL;
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    //事件分发
    it = gs_mapEventID_PFN.find(m_debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
    if (it != itend)
    {
        pfnEvent = (*it).second;
        dwContinueStatus = (this->*pfnEvent)();   
     }

    return dwContinueStatus;
}

DWORD
CUseDebugger::OnCreateThread()
{
    return m_pProcessEvent->OnCreateThread(this);
}

DWORD
CUseDebugger::OnCreateProcess()
{
    return m_pProcessEvent->OnCreateProcess(this);
}

DWORD
CUseDebugger::OnExitThread()
{
    return m_pProcessEvent->OnExitThread(this);
}

DWORD
CUseDebugger::OnExitProcess()
{
    return m_pProcessEvent->OnExitProcess(this);
}

DWORD
CUseDebugger::OnLoadDLL()
{    
    return m_pDllEvent->OnLoad(this);
}
DWORD
CUseDebugger::OnUnLoadDLL()
{
    return m_pDllEvent->OnUnload(this);
}

DWORD
CUseDebugger::OnOutputDebugString()
{
    return m_pDllEvent->OnOutputString(this);
}

DWORD
CUseDebugger::OnAccessViolation()
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    return dwContinueStatus;
}

DWORD
CUseDebugger::OnBreakPoint()
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    
    static BOOL bSysPoint = TRUE;
    if (bSysPoint)
    {
        m_bTalk = TRUE;
        bSysPoint = FALSE;
        dwContinueStatus = DBG_CONTINUE;
    }

    return dwContinueStatus;
}

DWORD
CUseDebugger::OnSingleStep()
{
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    return dwContinueStatus;
}

/************************************************************************/
/* 
Function : dispatch different input to different functions                                                                    */
/************************************************************************/
void
CUseDebugger::DispatchInput(int argc, int pargv[], const char *pszBuf)
{
   
}

void
CUseDebugger::DoStepOver(int argc, int pargv[], const char *pszBuf)
{

}

void
CUseDebugger::DoStepInto(int argc, int pargv[], const char *pszBuf)
{
    
}

void
CUseDebugger::DoRun(int argc, int pargv[], const char *pszBuf)
{
    
}

void
CUseDebugger::DoShowASM(int argc, int pargv[], const char *pszBuf)
{  
}

void
CUseDebugger::DoShowData(int argc, int pargv[], const char *pszBuf)
{  
}

void
CUseDebugger::DoShowRegs(int argc, int pargv[], const char *pszBuf)
{  
}

void
CUseDebugger::DoBP(int argc, int pargv[], const char *pszBuf)
{  
}

void
CUseDebugger::DoBPL(int argc, int pargv[], const char *pszBuf)
{  
}







