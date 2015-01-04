// UseDebugger.h: interface for the CUseDebugger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_USEDEBUGGER_H__2C1826AE_22DA_4F14_AA9D_8D88C3CC2A9F__INCLUDED_)
#define AFX_USEDEBUGGER_H__2C1826AE_22DA_4F14_AA9D_8D88C3CC2A9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DllEvent.h"
#include "ProcessEvent.h"
#include "ExceptEvent.h"

class CUseDebugger : public CBaseEvent
{
public:
    //the main entry for user to start
    static CUseDebugger *CreateSystem(void);

    virtual void Run(void);

    virtual void DestorySystem(void);

protected:
	CUseDebugger();
	virtual ~CUseDebugger();
    
    //two different ways to debug
    virtual BOOL DebugNewProcess();
    virtual BOOL DebugAttachedProcess();

    //the actual part to debug process
    virtual BOOL DebugProcess();

    //classified debug event
    virtual DWORD OnExceptDispatch();  //-->OnAccessViolation(), OnBreakPoint(), OnSingleStep()
    virtual DWORD OnCreateThread();    //-->CProcessEvent.
    virtual DWORD OnCreateProcess();   //-->CProcessEvent.
    virtual DWORD OnExitThread();      //-->CProcessEvent
    virtual DWORD OnExitProcess();     //-->CProcessEvent
    virtual DWORD OnLoadDLL();         //-->CDllEvent
    virtual DWORD OnUnLoadDLL();       //-->CDllEvent
    virtual DWORD OnOutputDebugString();//-->CDllEvent
    virtual DWORD OnAccessViolation();  //-->CExceptEvent
    virtual DWORD OnBreakPoint();       //-->CExceptEvent
    virtual DWORD OnSingleStep();       //-->CExceptEvent

    //classfied user input
    virtual BOOL DoShowData(int argc, int pargv[], const char *pszBuf); //-->CBaseEvent
    virtual BOOL DoShowASM(int argc, int pargv[], const char *pszBuf);  //-->CBaseEvent
    virtual BOOL DoShowRegs(int argc, int pargv[], const char *pszBuf); //-->CBaseEvent
    virtual BOOL DoShowHelp(int argc = NULL, int pargv[] = NULL, const char *pszBuf = NULL);
    virtual BOOL Quit(int argc, int pargv[], const char *pszBuf);

    virtual BOOL DoStepOver(int argc, int pargv[], const char *pszBuf); //-->CExceptEvent
    virtual BOOL DoStepInto(int argc, int pargv[], const char *pszBuf); //-->CExceptEvent
    virtual BOOL DoGo(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent
    virtual BOOL DoBPL(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent
    virtual BOOL DoBPC(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent

    //virtual BOOL DoBM(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent
    //virtual BOOL DoBM(int argc, int pargv[], const char *pszBuf, BOOL bTrace);       //-->CExceptEvent
    virtual BOOL DoBML(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent
    virtual BOOL DoBMPL(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent
    //virtual BOOL DoBMC(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent

    virtual BOOL DoBH(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent
    virtual BOOL DoBHL(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent
    virtual BOOL DoBHC(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent

    //about script export, import, operation log
    virtual BOOL DoExport(int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoImport(int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoLog(int argc, int pargv[], const char *pszBuf);

    //extended function
    virtual BOOL DoTrace(int argc, int pargv[], const char *pszBuf);
    //virtual BOOL DoShowSEH(int argc, int pargv[], const char *pszBuf);
    //virtual BOOL MonitorSEH(CBaseEvent *pEvent);
    virtual BOOL DoListModule(int argc, int pargv[], const char *pszBuf);

public:
    virtual BOOL DoBM(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent
    virtual BOOL DoBM(int argc, int pargv[], const char *pszBuf, BOOL bTrace);       //-->CExceptEvent
    virtual BOOL DoBMC(int argc, int pargv[], const char *pszBuf);      //-->CExceptEvent
    virtual BOOL DoBP(int argc, int pargv[], const char *pszBuf);       //-->CExceptEvent
    virtual BOOL RemoveTrace(tagModule *pModule);                       

    virtual BOOL MonitorSEH(CBaseEvent *pEvent);
    virtual BOOL DoShowSEH(int argc, int pargv[], const char *pszBuf);
    virtual BOOL ReadBuf(CBaseEvent *pEvent, HANDLE hProcess, LPVOID lpAddr, LPVOID lpBuf, SIZE_T nSize);

private:
    void DispatchCommand();

protected:
    CDllEvent *m_pDllEvent;
    CProcessEvent *m_pProcessEvent;
    CExceptEvent *m_pExceptEvent;
};

#endif // !defined(AFX_USEDEBUGGER_H__2C1826AE_22DA_4F14_AA9D_8D88C3CC2A9F__INCLUDED_)
