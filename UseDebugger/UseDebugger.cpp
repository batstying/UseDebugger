// UseDebugger.cpp: implementation of the CUseDebugger class.
//
//////////////////////////////////////////////////////////////////////
#include "UseDebugger.h"

BOOL gs_bContinue = TRUE;

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

//function pointer, used for debug event dispatch
typedef DWORD (CUseDebugger::*PFNDispatchEvent)(void);
static map<DWORD, PFNDispatchEvent> gs_mapEventID_PFN;

//for map<const char *, pfn>
class Compare
{
public:
    bool operator() (const char * pszSRC, const char * pszDST) const
    {
        return strcmp(pszSRC, pszDST) < 0;
    }
};

//used for input dispatch
typedef BOOL (CUseDebugger::*PFNDispatchInput)(int argc, int pargv[], const char *pszBuf);
static map<const char *, PFNDispatchInput, Compare> gs_mapInput_PFN;

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
    DISPATCHINPUT("bm",     CUseDebugger::DoBM)
    DISPATCHINPUT("bml",    CUseDebugger::DoBML)
    DISPATCHINPUT("bmpl",   CUseDebugger::DoBMPL) 
    DISPATCHINPUT("bmc",    CUseDebugger::DoBMC)
    DISPATCHINPUT("bp",     CUseDebugger::DoBP);
    DISPATCHINPUT("bpl",    CUseDebugger::DoBPL);
    DISPATCHINPUT("bpc",    CUseDebugger::DoBPC);
    DISPATCHINPUT("t",      CUseDebugger::DoStepInto);
    DISPATCHINPUT("g",      CUseDebugger::DoGo);
    DISPATCHINPUT("r",      CUseDebugger::DoShowRegs);
    DISPATCHINPUT("bh",     CUseDebugger::DoBH);
    DISPATCHINPUT("bhl",    CUseDebugger::DoBHL);
    DISPATCHINPUT("bhc",    CUseDebugger::DoBHC);
    DISPATCHINPUT("p",      CUseDebugger::DoStepOver);
    DISPATCHINPUT("u",      CUseDebugger::DoShowASM);
    DISPATCHINPUT("d",      CUseDebugger::DoShowData);
    DISPATCHINPUT("?",      CUseDebugger::DoShowHelp);
    DISPATCHINPUT("help",   CUseDebugger::DoShowHelp);
    //DISPATCHINPUT("q",      CUseDebugger::Quit);
    DISPATCHINPUT("es",     CUseDebugger::DoExport);
    DISPATCHINPUT("ls",     CUseDebugger::DoImport);
    DISPATCHINPUT("log",    CUseDebugger::DoLog);
    DISPATCHINPUT("trace",  CUseDebugger::DoTrace);
    DISPATCHINPUT("vseh",   CUseDebugger::DoShowSEH);
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

    m_pExceptEvent = NULL;
    m_pExceptEvent = new CExceptEvent;
    assert(m_pExceptEvent != NULL);

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

/************************************************************************/
/* 
Function : the main loop for our debugger
1) show main menu
2) debug event dispatch
3) interact with user
4) user input dispatch                                                  */
/************************************************************************/
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
            bRet = this->DebugAttachedProcess();  
        }
        else if (ch == '3')
        {
            bRet = this->DoShowHelp();
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
        CUI::ShowErrorMessage();
        return FALSE;
	}

    this->DebugProcess();

    return TRUE;
}

BOOL
CUseDebugger::DebugAttachedProcess()
{
    m_pUI->ShowInfo("Please Enter the PID:\r\n");
    system("taskmgr");

    //get pid
    int argc;
    int pargv[MAXBYTE];
    m_pUI->GetInput(&argc, pargv, g_szBuf, MAXBUF);

    DWORD dwPID = strtoul(g_szBuf, NULL, 10);
    assert(dwPID != 0 && dwPID != ULONG_MAX);

    if (DebugActiveProcess(dwPID))
    {
        this->DebugProcess();
        return TRUE;
    }
    
    return FALSE;
}

/************************************************************************/
/* 
Function : the main loop for our debugger
1) show main menu
2) debug event dispatch
3) interact with user
4) user input dispatch                                                  */
/************************************************************************/
BOOL
CUseDebugger::DebugProcess()
{
    //to fix error C2065: 'OpenThread' : undeclared identifier
    typedef HANDLE (WINAPI *OPENTHREAD)(DWORD, BOOL, DWORD);
    OPENTHREAD pfnOpenThread = (OPENTHREAD)GetProcAddress(GetModuleHandle("Kernel32"), "OpenThread");
    assert(pfnOpenThread != NULL);

    //used for input decode
    int argc;
    int pargv[MAXBYTE];

    //used for event dispatch
    map<DWORD, PFNDispatchEvent>::iterator itevt;
    map<DWORD, PFNDispatchEvent>::iterator itevtend = gs_mapEventID_PFN.end();
    PFNDispatchEvent pfnEvent = NULL;

    //used for input dispatch
    map<const char *, PFNDispatchInput, Compare>::iterator itinput;
    map<const char *, PFNDispatchInput, Compare>::iterator itinputend = gs_mapInput_PFN.end();
    PFNDispatchInput pfnInput = NULL;

    BOOL bRet       = TRUE;                      
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    while (gs_bContinue)
    {         
        bRet = ::WaitForDebugEvent(&m_debugEvent, INFINITE); 
        if (!bRet)
        {
            CUI::ShowErrorMessage();
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
            CUI::ShowErrorMessage();
            return FALSE;
        }
            
        m_hThread = pfnOpenThread(
                    THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION , 
                    FALSE,
                    m_debugEvent.dwThreadId
                    );
        if (NULL == m_hThread)
        {
            CUI::ShowErrorMessage();
            return FALSE;
        }
         
        m_Context.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
        bRet = ::GetThreadContext(m_hThread, &m_Context);
        if (!bRet)
        {
            CUI::ShowErrorMessage();
            return FALSE;
        }

        //just a trick
        this->m_Context.Dr7 |= DR7INIT;
        
        //debug event dispatch
        itevt = gs_mapEventID_PFN.find(m_debugEvent.dwDebugEventCode);
        if (itevt != itevtend)
        {
            pfnEvent = (*itevt).second;
            dwContinueStatus = (this->*pfnEvent)();   
        }

        //interact with the user
        while (m_bTalk)
        {
            m_pUI->GetInput(&argc, pargv, g_szBuf, MAXBUF);
            
            //user input dispatch
            itinput = gs_mapInput_PFN.find(g_szBuf);
            if (itinput != itinputend)
            {
                pfnInput = (*itinput).second;
                (this->*pfnInput)(argc, pargv, g_szBuf); 
            }
            else
            {
                m_pUI->ShowInfo("Invalid Input\r\n");
            }
        }

        //restore context, and close handles      
        bRet = ::SetThreadContext(m_hThread, &m_Context);
        if (!bRet)
        {
            CUI::ShowErrorMessage();
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

/************************************************************************/
/* 
Function : dispatch exception event                                                                     */
/************************************************************************/
DWORD
CUseDebugger::OnExceptDispatch()
{
    map<DWORD, PFNDispatchEvent>::iterator it;
    map<DWORD, PFNDispatchEvent>::iterator itend = gs_mapEventID_PFN.end();
    PFNDispatchEvent pfnEvent = NULL;
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    //dispatch
    it = gs_mapEventID_PFN.find(m_debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
    if (it != itend)
    {
        pfnEvent = (*it).second;
        dwContinueStatus = (this->*pfnEvent)();   
    }

    return dwContinueStatus;
}

/************************************************************************/
/* 
Function : all these used for event dispatch,
          dispatch into different event processing functions
1) Create(/Exit)Process(/Thread) --->CProcessEvent
2) Load(/Unload)Dll --> CDllEvent
3) DebugString --> CDllEvent
4) Exception (BreakPoint, AccessViolation, SingleStep) --> CExceptEvent*/
/************************************************************************/
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
    return m_pExceptEvent->OnAccessViolation(this);
}

DWORD
CUseDebugger::OnBreakPoint()
{
    return m_pExceptEvent->OnBreakPoint(this);
}

DWORD
CUseDebugger::OnSingleStep()
{
    //This is the debug event,  different from 't'
    return m_pExceptEvent->OnSingleStep(this);
}

/************************************************************************/
/* 
Function : User input dispatch
1) ShowASM, ShowData, ShowRegs --> CBaseEvent
2) others (BP, BPL, BPC, BM, BH .etc) ---> CExceptEvent
    put exception event, break point set function together
    try to get easy maintainment                                      */
/************************************************************************/
BOOL
CUseDebugger::DoShowASM(int argc, int pargv[], const char *pszBuf)
{ 
    //u
    m_bTalk = TRUE;
    return m_pExceptEvent->DoShowASM(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoShowData(int argc, int pargv[], const char *pszBuf)
{ 
    //d
    m_bTalk = TRUE;

    m_pExceptEvent->DoShowData(this, argc, pargv, pszBuf);
    return TRUE;
}

BOOL
CUseDebugger::DoShowRegs(int argc, int pargv[], const char *pszBuf)
{
    //r
    m_bTalk = TRUE;
    m_pExceptEvent->DoShowRegs(this);
    return TRUE;
}

BOOL
CUseDebugger::DoShowHelp(int argc/*=NULL*/, int pargv[]/*=NULL*/, const char *pszBuf/*=NULL*/)
{
    static char szBuf[1024];
    _snprintf(szBuf, 1024, "----------------帮助-----------------\r\n"
                            "命令   格式                作用\r\n"  
                            "bm     bm addr a|w len     内存断点设置\r\n"			 
                            "bml    bml                 内存断点查看\r\n"
                            "bmpl   bmpl                分页内内存断点查看\r\n"
                            "bmc    bmc id (from bml)   硬件断点删除\r\n"
                            "bp     bp addr             一般断点设置\r\n"
                            "bpl    bpl                 一般断点查看\r\n"
                            "bpc    bpc id (from bpl)   一般断点删除\r\n"
                            "bh     bh addr e|w|a 1|2|4 硬件断点设置\r\n"
                            "bhl    bhl                 硬件断点查看\r\n"
                            "bhc    bhc id (from bhl)   硬件断点删除\r\n"
                            "t      t                   步入        \r\n"
                            "p      p                   步过\r\n"
                            "g      g [addr]            运行\r\n"
                            "r      r                   寄存器查看\r\n"
                            "u      u [addr]            汇编查看\r\n"
                            "d      d [addr]            内存数据查看\r\n"
                            //"q      q                   退出当前调试\r\n"
                            "?      ?                   帮助\r\n"
                            "help   help                帮助\r\n"
                            "log    log                 记录所有\r\n"
                            );

    m_pUI->ShowInfo(szBuf);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
BOOL
CUseDebugger::DoStepOver(int argc, int pargv[], const char *pszBuf)
{
    //p
    m_bTalk = FALSE;
    return m_pExceptEvent->DoStepOver(this/*, argc, pargv, pszBuf*/);
}

BOOL
CUseDebugger::DoStepInto(int argc, int pargv[], const char *pszBuf)
{
    //t
    m_bUserTF = TRUE;
    m_bTalk = FALSE;
    return m_pExceptEvent->DoStepInto(this/*, argc, pargv, pszBuf*/);    
}

BOOL
CUseDebugger::DoGo(int argc, int pargv[], const char *pszBuf)
{
    //g
    m_bTalk = FALSE;
    return m_pExceptEvent->DoGo(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBP(int argc, int pargv[], const char *pszBuf)
{ 
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBP(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBPL(int argc, int pargv[], const char *pszBuf)
{ 
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBPL(this/*, argc, pargv, pszBuf*/);
}

BOOL
CUseDebugger::DoBPC(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBPC(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBM(int argc, int pargv[], const char *pszBuf)
{ 
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBM(this, argc, pargv, pszBuf, FALSE);
}

BOOL
CUseDebugger::DoBML(int argc, int pargv[], const char *pszBuf)
{  
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBML(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBMPL(int argc, int pargv[], const char *pszBuf)
{  
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBMPL(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBMC(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBMC(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBH(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBH(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::DoBHL(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBHL(this/*,argc, pargv, pszBuf*/);
}

BOOL 
CUseDebugger::DoBHC(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoBHC(this, argc, pargv, pszBuf);
}

BOOL
CUseDebugger::Quit(int argc, int pargv[], const char *pszBuf)    
{
    m_bTalk = FALSE;
    gs_bContinue = FALSE;
    return TRUE;
}

/************************************************************************/
/* 
Function : save all the user input (during debugging) into file                                                                      */
/************************************************************************/
BOOL
CUseDebugger::DoExport(int argc, int pargv[], const char *pszBuf)
{
    m_pUI->ExportScript();
    return TRUE;
}

/************************************************************************/
/* 
Function : load the saved script(command history) and execute           */
/************************************************************************/
BOOL 
CUseDebugger::DoImport(int argc, int pargv[], const char *pszBuf)
{
    m_pUI->ImportScript();
    return TRUE;
} 

/************************************************************************/
/* 
Function : save all the operations, outputs, whatever you can see on screen 
            into file in time                                            */
/************************************************************************/
BOOL 
CUseDebugger::DoLog(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    m_pUI->Log();
    return TRUE;
}

BOOL 
CUseDebugger::DoTrace(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    m_pUI->Trace();
    return m_pExceptEvent->DoTrace(this, argc, pargv, pszBuf);
}

BOOL 
CUseDebugger::DoShowSEH(int argc, int pargv[], const char *pszBuf)
{
    m_bTalk = TRUE;
    return m_pExceptEvent->DoShowSEH(this, argc, pargv, pszBuf);
}

BOOL 
CUseDebugger::MonitorSEH(CBaseEvent *pEvent)
{
    return m_pExceptEvent->MonitorSEH(pEvent);
}






