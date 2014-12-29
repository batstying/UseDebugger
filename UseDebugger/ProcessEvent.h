// ProcessEvent.h: interface for the CProcessEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCESSEVENT_H__FAFADB88_BC73_446F_BE60_76C8F21085E9__INCLUDED_)
#define AFX_PROCESSEVENT_H__FAFADB88_BC73_446F_BE60_76C8F21085E9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEvent.h"

/************************************************************************/
/* Deal with Process (Thead) Related, like Create, Exit,                                                                  */
/************************************************************************/
class CProcessEvent : public CBaseEvent  
{
public:
	DWORD OnExitProcess(const CBaseEvent *pEvent);
	DWORD OnExitThread(const CBaseEvent *pEvent);
	DWORD OnCreateThread(const CBaseEvent *pEvent);
	DWORD OnCreateProcess(CBaseEvent *pEvent);
	CProcessEvent();
	virtual ~CProcessEvent();

};

#endif // !defined(AFX_PROCESSEVENT_H__FAFADB88_BC73_446F_BE60_76C8F21085E9__INCLUDED_)
