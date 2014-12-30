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
//�������ڴ�ϵ�
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

//����ҳ�����ض�λ��Ľṹ��ά��ҳ�ڶϵ���Ϣ
//�ڴ�ϵ��ڷ�ҳ�ڵı�ʾ
typedef struct _tagMemBPInPage 
{
    WORD wOffset;       //��ҳ�ڵ�ƫ��
    WORD wSize;         //��ҳ�ڵĴ�С
    bool operator == (const _tagMemBPInPage &obj)
    {
        return ((wOffset == obj.wOffset)
                && (wSize == obj.wSize)
                );
    }
}tagMemBPInPage;

//��ҳ��ϵ�
typedef struct _tagPageBP			
{
    DWORD dwPageAddr;   //��ҳ��ַ
    DWORD dwOldProtect;     
    DWORD dwNewProtect;
    list<tagMemBPInPage> lstMemBP;
}tagPageBP;

//////////////////////////////////////////////////////////////////////////
//Structures about Normal BreakPoint
//�����ΪNormalBP��һ��ϵ㣬����˵��INT3������CC������Ϊ���ǵ��ֽڵ���Ȩָ���������������
typedef struct _tagNormalBP   
{
    byte oldvalue;        //ԭ�ֽ�
    byte bTmp:1;          //��ʱ�ԣ�Debugger�ڲ�����
    byte bPerment: 1;     //�û�ͨ��bp����
    //byte bEnabled: 1;     //�û�ͨ��bd, be���ã��Ա�disable, enable�ϵ�
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
    list<tagMemBP> m_lstMemBP;                 //�������ڴ�ϵ�
    map<DWORD, tagPageBP> m_mapPage_PageBP;    //����ҳά���Ķϵ�

    map<DWORD, tagNormalBP> m_mapAddr_NormBP;  //һ��ϵ�

    DWORD m_dwPageSize;
};

#endif // !defined(AFX_EXCEPTEVENT_H__2B93A038_E851_4B9A_9F4E_19619B6852D6__INCLUDED_)
