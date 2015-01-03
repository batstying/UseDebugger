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
    BOOL  bTrace;        //used for trace
#define MEMBP_ACCESS 0   //R, W
#define MEMBP_WRITE  1
    DWORD dwType;        //write or  access ;
    bool operator == (const _tagMemBP &obj)
    {
        return ((dwAddr == obj.dwAddr)
            && (dwSize == obj.dwSize)
            && (dwType == obj.dwType)
            && (bTrace == obj.bTrace)
            );
    }
}tagMemBP;

//各分页按照重定位表的结构来维护页内断点信息
//内存断点在分页内的表示
typedef struct _tagMemBPInPage 
{
    WORD wOffset;       //在页内的偏移
    WORD wSize;         //在页内的大小
    BOOL bTrace;        //used for trace
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
    byte bDisabled: 1;     //用于处理对int 3设一般断点
}tagNormalBP;

//////////////////////////////////////////////////////////////////////////
//Structures about HardWare
typedef struct _tagDR7
{
    unsigned /*char*/ GL0: 2;
    unsigned /*char*/ GL1: 2;
    unsigned /*char*/ GL2: 2;
    unsigned /*char*/ GL3: 2;
    unsigned /*char*/ GLE: 2;     // 11
    unsigned /*char*/ Reserv0: 3; // 001
    unsigned /*char*/ GD : 1;     // 0
    unsigned /*char*/ Reserv1: 2; //00
    unsigned /*char*/ RW0: 2;
    unsigned /*char*/ LEN0: 2;
    unsigned /*char*/ RW1: 2;
    unsigned /*char*/ LEN1: 2;
    unsigned /*char*/ RW2: 2;
    unsigned /*char*/ LEN2: 2;
    unsigned /*char*/ RW3: 2;
    unsigned /*char*/ LEN3: 2;
#define DR7INIT 0x00000700  //Reserv1:00 GD:0 Reserv0:001  GELE:11
}tagDR7;

typedef struct _tagDR6
{
    unsigned /*char*/ B0:1;
    unsigned /*char*/ B1:1;
    unsigned /*char*/ B2:1;
    unsigned /*char*/ B3:1;
    unsigned /*char*/ Reserv0:8;      //11111111
    unsigned /*char*/ Reserv1:1;    //0
    unsigned /*char*/ BD:1;
    unsigned /*char*/ BS:1;
    unsigned /*char*/ BT:1;
    unsigned /*char*/ Reserv2:16;              //set to 1
}tagDR6;

typedef struct _tagHWBP
{
    DWORD dwAddr;
    DWORD dwType;
    DWORD dwLen;
    DWORD *pDRAddr[4];      //for DR0 ~ DR3
    DWORD RW[4];          //for DR7:RW0 ~ RW3
#define HWBP_EXECUTE 0  //break on instruction execution only
#define HWBP_WRITE   1  //break on data writes only
#define HWBP_ACCESS  3  //break on data reads or write but not instruction fetches
#define STREXECUTE  "Execute"
#define STRWRITE    "Write"
#define STRACCESS   "Access"
}tagHWBP;

//////////////////////////////////////////////////////////////////////////
//see IA1.pdf 3.4.3 EFLAG registers
typedef struct _tagEFlags
{
    unsigned /*char*/ CF:1;
    unsigned /*char*/ Reserv1:1; //1
    unsigned /*char*/ PF:1;
    unsigned /*char*/ Reserv2:1; //0
    unsigned /*char*/ AF:1; 
    unsigned /*char*/ Reserv3:1; //0
    unsigned /*char*/ ZF:1;
    unsigned /*char*/ SF:1;
    unsigned /*char*/ TF:1;
    unsigned /*char*/ IF:1;
    unsigned /*char*/ DF:1;
    unsigned /*char*/ OF:1;
    //others
    unsigned /*char*/ IOPL:2;
    unsigned /*char*/ NT:1;
    unsigned /*char*/ Reserv4:1; //0
    unsigned /*char*/ Remain:16;
}tagEFlags;

//////////////////////////////////////////////////////////////////////////
//about seh
typedef struct _tagSEH
{
    DWORD ptrNext;      //pointer to next seh record
    DWORD dwHandler;    //SEH handler
}tagSEH;

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
    virtual BOOL DoStepOver(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/); 
    virtual BOOL DoStepInto(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/); 
    virtual BOOL DoGo(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);      
    virtual BOOL DoBP(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);       
    virtual BOOL DoBPL(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/);
    virtual BOOL DoBPC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);       
    
    virtual BOOL DoBM(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf, BOOL bTrace);       
    virtual BOOL DoBML(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoBMPL(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoBMC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);

    virtual BOOL DoBH(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoBHL(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/); 
    virtual BOOL DoBHC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);

    //show
    virtual BOOL DoShowData(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf); 
    virtual BOOL DoShowASM(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);
    virtual const char * ShowOneASM(CBaseEvent *pEvent, DWORD dwAddr = NULL, UINT *pnCodeSize = NULL);
    virtual void ShowTwoASM(CBaseEvent *pEvent, DWORD dwAddr = NULL); 
    virtual void DoShowRegs(CBaseEvent *pEvent); 

    //extended function
    virtual BOOL DoTrace(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);
    virtual BOOL DoShowSEH(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf);
    virtual BOOL MonitorSEH(CBaseEvent *pEvent);

    //
    DWORD GetTIB(CBaseEvent *pEvent);

protected:
    //
    BOOL CheckHitMemBP(CBaseEvent *pEvent, DWORD dwAddr, tagPageBP *ppageBP);
    BOOL CheckBMValidity(CBaseEvent *pEvent, tagMemBP *pMemBP);
    BOOL IsPageValid(CBaseEvent *pEvent, DWORD dwAddr);
    BOOL HasMemBP(CBaseEvent *pEvent, DWORD dwAddr, tagPageBP **ppPageBP);
    BOOL HasNormalBP(CBaseEvent *pEvent, DWORD dwAddr, tagNormalBP **ppNormalBP);
    BOOL HasOtherMemBP(CBaseEvent *pEvent, DWORD dwPageAddr, tagPageBP **ppPageBP, DWORD *pnTotal);
    BOOL SetHWBP(CBaseEvent *pEvent, tagHWBP *pHWBP);
    BOOL HasHitHWBP(CBaseEvent *pEvent);
    
protected:
    list<tagMemBP> m_lstMemBP;                 //独立的内存断点
    map<DWORD, tagPageBP> m_mapPage_PageBP;    //各分页维护的断点

    map<DWORD, tagNormalBP> m_mapAddr_NormBP;  //一般断点

    DWORD m_dwPageSize;
};

#endif // !defined(AFX_EXCEPTEVENT_H__2B93A038_E851_4B9A_9F4E_19619B6852D6__INCLUDED_)
