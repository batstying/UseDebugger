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
    /************************************************************************/
    /* 
    Function : display registers and asm code pointed by eip               */
    /************************************************************************/
    virtual void ShowRegs();
    
    virtual void ShowOneASM();

public:
    CUI *m_pUI;
    HANDLE m_hProcess;
    HANDLE m_hThread;
    DEBUG_EVENT m_debugEvent;
    CONTEXT m_Context;
    BOOL m_bTalk;           //whether interact with the user
};

#endif // !defined(AFX_BASEEVENT_H__D36850DA_9A3E_4F74_AB94_05BB537A72D8__INCLUDED_)
