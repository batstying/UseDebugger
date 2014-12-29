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
    virtual void DoShowData(int argc, int pargv[], const char *pszBuf); 
    virtual void DoShowASM(int argc, int pargv[], const char *pszBuf);
    virtual void ShowOneASM();
    virtual void DoShowRegs();                                          
    

public:
    CUI *m_pUI;
    HANDLE m_hProcess;
    HANDLE m_hThread;
    DWORD  m_dwOEP;
    DEBUG_EVENT m_debugEvent;
    CONTEXT m_Context;
    BOOL m_bTalk;           //whether interact with the user

    DWORD m_dwAddr;         //the addr need to deal with
    BOOL  m_bAccessVioTF;   //single step because of AccessViolation
};

#endif // !defined(AFX_BASEEVENT_H__D36850DA_9A3E_4F74_AB94_05BB537A72D8__INCLUDED_)
