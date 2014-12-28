// DllEvent.h: interface for the CDllEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DLLEVENT_H__186E4F6C_0E00_4D23_A7C7_F70A76F94E62__INCLUDED_)
#define AFX_DLLEVENT_H__186E4F6C_0E00_4D23_A7C7_F70A76F94E62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseEvent.h"

/************************************************************************/
/* Deal with Events about Dll load, unload 
   also include OutPutDebugString                                      */
/************************************************************************/
class CDllEvent : public CBaseEvent  
{
public:
	CDllEvent();
	virtual ~CDllEvent();

public:
    DWORD OnOutputString(const CBaseEvent *pEvent);
    DWORD OnUnload(const CBaseEvent *pEvent);
	DWORD OnLoad(const CBaseEvent *pBaseEvent);
    BOOL GetImageName(const CBaseEvent *pBaseEvent);
};

#endif // !defined(AFX_DLLEVENT_H__186E4F6C_0E00_4D23_A7C7_F70A76F94E62__INCLUDED_)
