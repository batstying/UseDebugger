// ExceptEvent.h: interface for the CExceptEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCEPTEVENT_H__2B93A038_E851_4B9A_9F4E_19619B6852D6__INCLUDED_)
#define AFX_EXCEPTEVENT_H__2B93A038_E851_4B9A_9F4E_19619B6852D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/************************************************************************/
/* 
Deal with Exception : Break Point, Single Step, Access Violation
      and Hardware Break Point, Memory Break Point                      */
/************************************************************************/

#include "BaseEvent.h"

//////////////////////////////////////////////////////////////////////////
//Structures about memory breakpoint
//独立的内存断点
typedef struct _tagMemBP    
{
    DWORD dwAddr;
    DWORD dwSize;
#define MEMBP_ACCESS 0   //R, W
#define MEMBP_WRITE  1
    DWORD dwType;        //write or  access ;
    bool operator == (const _tagMemBP &obj)
    {
        return ((dwAddr == obj.dwAddr)
            && (dwSize == obj.dwSize)
            && (dwType == obj.dwType)
            );
    }
}tagMemBP;

//各分页按照重定位表的结构来维护页内断点信息
//内存断点在分页内的表示
typedef struct _tagMemBPInPage 
{
    WORD wOffset;       //在页内的偏移
    WORD wSize;         //在页内的大小
    bool operator == (const _tagMemBPInPage &obj)
    {
        return ((wOffset == obj.wOffset)
                && (wSize == obj.wSize)
                );
    }
}tagMemBPInPage;

//分页与断点
typedef struct _tagPageBP			
{
    DWORD dwPageAddr;   //分页地址
    DWORD dwOldProtect;     
    DWORD dwNewProtect;
    list<tagMemBPInPage> lstMemBP;
}tagPageBP;

//////////////////////////////////////////////////////////////////////////
//Structures about Normal BreakPoint
//这里称为NormalBP，一般断点，而不说是INT3，或者CC，是因为凡是单字节的特权指令，都可以用来处理。
typedef struct _tagNormalBP   
{
    byte oldvalue;        //原字节
    byte bTmp:1;          //临时性，Debugger内部设置
    byte bPerment: 1;     //用户通过bp设置
    //byte bEnabled: 1;     //用户通过bd, be设置，以便disable, enable断点
}tagNormalBP;

//////////////////////////////////////////////////////////////////////////
class CExceptEvent : public CBaseEvent  
{
public:
	CExceptEvent();
	virtual ~CExceptEvent();

public:	
    //event
    virtual DWORD OnAccessViolation(CBaseEvent *pEvent);
    virtual DWORD OnBreakPoint(CBaseEvent *pEvent);
    virtual DWORD OnSingleStep(CBaseEvent *pEvent);

    //user input
    virtual BOOL DoStepOver(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoStepInto(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/); 
    virtual BOOL DoGo(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);      
    virtual BOOL DoBP(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);       
    virtual BOOL DoBPL(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);
    
    virtual BOOL DoBM(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);       
    virtual BOOL DoBML(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoBMPL(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoBMC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);
    
    //
    BOOL CheckBMValidity(CBaseEvent *pEvent, DWORD dwAddr, DWORD dwType, DWORD dwSize);
    BOOL IsPageValid(CBaseEvent *pEvent, DWORD dwAddr);
    BOOL HasMemBP(CBaseEvent *pEvent, DWORD dwAddr, tagPageBP **ppPageBP);
    BOOL HasNormalBP(CBaseEvent *pEvent, DWORD dwAddr, tagNormalBP **ppNormalBP);
    BOOL HasOtherMemBP(CBaseEvent *pEvent, DWORD dwPageAddr, tagPageBP **ppPageBP);
    
protected:
    list<tagMemBP> m_lstMemBP;                 //独立的内存断点
    map<DWORD, tagPageBP> m_mapPage_PageBP;    //各分页维护的断点

    map<DWORD, tagNormalBP> m_mapAddr_NormBP;  //一般断点

    DWORD m_dwPageSize;
};

#endif // !defined(AFX_EXCEPTEVENT_H__2B93A038_E851_4B9A_9F4E_19619B6852D6__INCLUDED_)
