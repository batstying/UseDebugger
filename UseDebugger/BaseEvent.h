// BaseEvent.h: interface for the CBaseEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASEEVENT_H__D36850DA_9A3E_4F74_AB94_05BB537A72D8__INCLUDED_)
#define AFX_BASEEVENT_H__D36850DA_9A3E_4F74_AB94_05BB537A72D8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UI.H"

class CBaseEvent  
{
public:
	CBaseEvent();
	virtual ~CBaseEvent();

public:
    virtual BOOL DoShowData(int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoShowASM(int argc, int pargv[], const char *pszBuf);
    virtual void DoShowRegs();  
    virtual void ShowOneASM();
    virtual void ShowTwoASM();                                        
    

public:
    CUI *m_pUI;
    HANDLE m_hProcess;
    HANDLE m_hThread;
    DWORD  m_dwOEP;
    DEBUG_EVENT m_debugEvent;
    CONTEXT m_Context;
    BOOL m_bTalk;           //whether interact with the user

    DWORD m_dwAddr;         //the addr need to deal with

    BOOL  m_bAccessVioTF;   //single step for AccessViolation
    BOOL  m_bNormalBPTF;    //single step for NormalBP
    BOOL  m_bUserTF;        //step into by user
    BOOL  m_bTmpBP;         //tmp NormalBP set by the Debugger self.
};

#endif // !defined(AFX_BASEEVENT_H__D36850DA_9A3E_4F74_AB94_05BB537A72D8__INCLUDED_)
