// ExceptEvent.cpp: implementation of the CExceptEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "ExceptEvent.h"

static const unsigned char gs_BP = 0xCC;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExceptEvent::CExceptEvent()
{
    SYSTEM_INFO  sysInfo;
    GetSystemInfo(&sysInfo);
    m_dwPageSize = sysInfo.dwPageSize;
}

CExceptEvent::~CExceptEvent()
{

}

/************************************************************************/
/* 
Function : 
Process  : take care of memory breakpoint
         1) whether memory breakpoints exists within the page  
         2) if yes, restore protect
                    set single step to change back                                                                 */
/************************************************************************/
DWORD
CExceptEvent::OnAccessViolation(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);

    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED; 
    EXCEPTION_DEBUG_INFO exceptInfo = pEvent->m_debugEvent.u.Exception;
    EXCEPTION_RECORD exceptRecord = exceptInfo.ExceptionRecord;
    DWORD dwAddr = exceptRecord.ExceptionInformation[1];

    //whether exists memory BP
    tagPageBP *ppageBP = NULL;
    DWORD dwOldProtect;
    BOOL bRet = HasMemBP(pEvent, dwAddr, &ppageBP);
    if (bRet)
    {
        //need to restore the protect, (and add PAGE_READWRITE)
        bRet = VirtualProtectEx(pEvent->m_hProcess,
                                (LPVOID)dwAddr,
                                sizeof(gs_BP),
                                ppageBP->dwOldProtect,
                                &dwOldProtect
                                );
        if (!bRet)
        {
            ShowErrorMessage();
            return DBG_CONTINUE;
        }

        //need to set single step to restore the protect
        m_bAccessVioTF = TRUE;
        m_dwAddr       = dwAddr;

        strcpy(g_szBuf, "t");
        int argv[] = {0};
        DoStepInto(pEvent, 1, argv, g_szBuf);
    }

    return dwContinueStatus;
}

DWORD
CExceptEvent::OnBreakPoint(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    
    static BOOL bSysPoint = TRUE;
    if (bSysPoint)
    {
        pEvent->m_bTalk = TRUE;
        bSysPoint = FALSE;
        dwContinueStatus = DBG_CONTINUE;
    }
    
    return dwContinueStatus;
}

DWORD
CExceptEvent::OnSingleStep(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    
    tagPageBP *ppageBP = NULL;
    DWORD dwOldProtect;
    BOOL bRet;
    if (m_bAccessVioTF)
    {
        bRet = HasMemBP(pEvent, m_dwAddr, &ppageBP);
        if (bRet)
        {
            //need to restore the protect (PAGE_NONACCESS)
            bRet = VirtualProtectEx(pEvent->m_hProcess,
                                    (LPVOID)m_dwAddr,
                                    sizeof(gs_BP),
                                    ppageBP->dwNewProtect,
                                    &dwOldProtect
                                    );
            if (!bRet)
            {
                ShowErrorMessage();
                return DBG_CONTINUE;
            }
        }
        m_bAccessVioTF = FALSE;
        return DBG_CONTINUE;
    }

    return dwContinueStatus;
}

//////////////////////////////////////////////////////////////////////////
BOOL
CExceptEvent::DoStepOver(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    return TRUE;
}

BOOL
CExceptEvent::DoStepInto(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    pEvent->m_Context.EFlags |= 0x100;
    return TRUE;  
}

BOOL
CExceptEvent::DoGo(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    return TRUE;    
}

/************************************************************************/
/* 
Function : judge whether the page where the addr in is valid
Params   : dwAddr is the specified address
Return   : TRUE is valid, FALSE otherwise                               */
/************************************************************************/
BOOL 
CExceptEvent::IsPageValid(CBaseEvent *pEvent, DWORD dwAddr)
{
    assert(pEvent != NULL);

    MEMORY_BASIC_INFORMATION memInfo;
    VirtualQueryEx(pEvent->m_hProcess,
                (LPVOID)dwAddr,
                &memInfo,
                sizeof(MEMORY_BASIC_INFORMATION)
                );
    
    if (memInfo.State != MEM_COMMIT)
    {
        return FALSE;
    }

    return TRUE;
}

/************************************************************************/
/* 
Function : judge whether the page where the addr in exists memory breakpoint
Params   : dwAddr is the specified address
           [OUT] pPageBP used to receive the pageBP info (if exists)
Return   : TRUE if exists, FALSE otherwise                                                                     */
/************************************************************************/
BOOL
CExceptEvent::HasMemBP(CBaseEvent *pEvent, DWORD dwAddr, tagPageBP **ppPageBP)
{
    assert(pEvent != NULL); 

    DWORD dwPageAddr = (dwAddr / m_dwPageSize) * m_dwPageSize;
    map<DWORD, tagPageBP>::iterator it;
    it = m_mapPage_PageBP.find(dwPageAddr);
    if (it != m_mapPage_PageBP.end())
    {
        *ppPageBP = &(*it).second;
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/* 
Function : judge whether the addr alread been set NormalBP
Params   : dwAddr is the specified address
Return   : TRUE if already been , FALSE otherwise                                                                    */
/************************************************************************/
BOOL
CExceptEvent::HasNormalBP(CBaseEvent *pEvent, DWORD dwAddr)
{
    assert(pEvent != NULL);

    map<DWORD, tagNormalBP>::iterator it;
    it = m_mapAddr_NormBP.find(dwAddr);
    if (it != m_mapAddr_NormBP.end())
    {
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/* 
Function : set breakpoint at the specified address
Process  :
         1) whether already been set
         2) whether the page valid 
         3) consider the page may be changed by Memory Point
                                                                     */
/************************************************************************/
BOOL
CExceptEvent::DoBP(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{  
    //bp addr
    assert(2 == argc);
    assert(pEvent != NULL);

    BOOL bRet;
    DWORD dwAddr = strtoul(&pszBuf[pargv[1]], NULL, 16);

    //whether already been set
    bRet = HasNormalBP(pEvent, dwAddr);
    if (bRet)
    {
        return FALSE;
    }

    //whether the page valid
    bRet = IsPageValid(pEvent, dwAddr);
    if (!bRet)
    {
        return FALSE;
    }

    //whether memory breakpoint exists within the page
    tagPageBP *ppageBP = NULL;
    DWORD dwOldProtect;
    bRet = HasMemBP(pEvent, dwAddr, &ppageBP);
    if (bRet)
    {
        //need to restore the protect, (and add PAGE_READWRITE)
        bRet = VirtualProtectEx(pEvent->m_hProcess,
                                (LPVOID)dwAddr,
                                sizeof(gs_BP),
                                ppageBP->dwOldProtect,
                                &dwOldProtect
                                );
        if (!bRet)
        {
            ShowErrorMessage();
            return FALSE;
        }
    }

    //now save the NormalBP
    tagNormalBP normalBP = {0};
    bRet = ReadProcessMemory(pEvent->m_hProcess,
                            (LPVOID)dwAddr,
                            (LPVOID)&normalBP.oldvalue,
                            sizeof(normalBP.oldvalue),
                            NULL);
    if (!bRet)
    {
        ShowErrorMessage();
        return FALSE;
    }

    bRet = WriteProcessMemory(pEvent->m_hProcess,
                        (LPVOID)dwAddr,
                        (LPVOID)&gs_BP,
                        sizeof(gs_BP),
                        NULL
                        );
    if (!bRet)
    {
        ::ShowErrorMessage();
        return FALSE;
    }

    //now save the NormalBP
    normalBP.bPerment = 0;
    normalBP.bTmp     = 1;
    normalBP.bEnabled = 1;  //not necessary
    m_mapAddr_NormBP[dwAddr] = normalBP;

    //restore the protect
    if (ppageBP != NULL)
    {
        bRet = VirtualProtectEx(pEvent->m_hProcess,
                            (LPVOID)dwAddr,
                            sizeof(gs_BP),
                            ppageBP->dwNewProtect,
                            &dwOldProtect
                            );
    }

    return TRUE;
}

BOOL
CExceptEvent::DoBPL(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    return TRUE;
}

/************************************************************************/
/* 
Function : check Memory BreakPoint validity  
Params   : pEvents  contains hProcess
           dwAddr  indicate the start point
           dwType  can be MEMBP_ACCESS, MEMBP_READ 
           dwSize 

Return   : TRUE is valid, and set some info
           Otherwise, FALSE 

Process :  
        1) if already exist
        2) stop if not MEM_COMMIT
        3) whether already been set (memory breakpoint already exists)
        4) whether necessary to set (like no need to set WRITE if it is READONLY
        */
/************************************************************************/
BOOL 
CExceptEvent::CheckBMValidity(CBaseEvent *pEvent, 
                              DWORD dwAddr,
                              DWORD dwType,
                              DWORD dwSize)
{
    assert(pEvent != NULL);

    //how many pages may be involed
    DWORD nPages = (dwAddr + dwSize) / m_dwPageSize - dwAddr / m_dwPageSize + 1;

    //check these memory state 
    MEMORY_BASIC_INFORMATION memInfo;
    tagMemBP       memBP;               //独立内存断点
    tagMemBPInPage memBPInPage;         //断点在分页内信息
    map<DWORD, tagPageBP>::iterator it;
    map<DWORD, tagPageBP>::iterator itend = m_mapPage_PageBP.end();
    list<tagMemBP>::iterator itMemBP;
    DWORD  dwPageAddr = (dwAddr / m_dwPageSize) * m_dwPageSize;
    DWORD dwOldProtect;
    BOOL bRet;

    //if already exist
    memBP.dwAddr = dwAddr;
    memBP.dwSize = dwSize;
    memBP.dwType = dwType;
    itMemBP = find(m_lstMemBP.begin(), m_lstMemBP.end(), memBP);
    if (itMemBP != m_lstMemBP.end())
    {
        return FALSE;
    }

    for (DWORD i = 0; i < nPages; i++)
    {
        VirtualQueryEx(pEvent->m_hProcess,
                        (LPVOID)dwPageAddr,
                        &memInfo,
                        sizeof(MEMORY_BASIC_INFORMATION)
                        );

        //not deal with MEM_FREE, MEM_RESERVE
        if (memInfo.State != MEM_COMMIT)
        {
            pEvent->m_pUI->ShowInfo("not MEM_COMMIT\r\n");
            break;
        }

        //if protect already set
        if (PAGE_NOACCESS == memInfo.Protect)
        {
            it = m_mapPage_PageBP.find(dwPageAddr);
            if (it == itend)
            {
                continue;
            }
            memInfo.Protect = (*it).second.dwOldProtect;
        }

        //if no need to set
        if ((MEMBP_WRITE == dwType)
            && (PAGE_READONLY == dwType
                || PAGE_EXECUTE == dwType
                || PAGE_EXECUTE_READ == dwType)
                //others?
                )
        {
            continue;
        }
       
        //can be more beautiful
        if (i > 0
            && i < nPages - 1)
        {
            memBPInPage.wOffset = 0;
            memBPInPage.wSize   = m_dwPageSize;
        }
        else if (0 == i)
        {
            memBPInPage.wOffset = dwAddr - dwPageAddr;
            memBPInPage.wSize   = min(dwSize, m_dwPageSize - memBPInPage.wOffset);
        }
        else    //i = nPages - 1
        {
            memBPInPage.wOffset = 0;
            memBPInPage.wSize   = dwAddr + dwSize - dwPageAddr;
        }

        m_mapPage_PageBP[dwPageAddr].dwPageAddr   = dwPageAddr;
        m_mapPage_PageBP[dwPageAddr].dwOldProtect = memInfo.Protect;
        m_mapPage_PageBP[dwPageAddr].dwNewProtect = PAGE_NOACCESS;
        m_mapPage_PageBP[dwPageAddr].lstMemBP.remove(memBPInPage);    //to avoid already exists
        m_mapPage_PageBP[dwPageAddr].lstMemBP.push_back(memBPInPage);

        //now change the protect
        bRet = VirtualProtectEx(pEvent->m_hProcess,
                            (LPVOID)dwAddr,
                            1,
                            PAGE_NOACCESS,
                            &dwOldProtect
                             );
        if (!bRet)
        {
            ShowErrorMessage();
        }

        dwPageAddr += m_dwPageSize;        
    } 

    //valid, just take it
    if (i != 0)
    {
        memBP.dwAddr = dwAddr;
        memBP.dwSize = dwSize;
        memBP.dwType = dwType;
        m_lstMemBP.push_back(memBP);
    }

    return TRUE;
}

/************************************************************************/
/* 
Function : set memory breakpoint   
Params   :  bm addr a|w len 
            addr为断点起始值，a|w分别表示访问类型和写入类型，len表示断点的长度
Process: 
1)断点合法性检查（分页是否有效，断点属性与分页属性，重复性设置），
2)新属性的设置，
3)分页断点的信息更新 

内存断点与分页关系的维护： 
1) 考虑模仿重定位表来维护分页内断点信息。
2) 用户输入的内存断点信息，独立的保存 （仅为显示所用）
3) 相关的分页中同时也维护涉及到的断点信息 （触发时的处理）
                                                                 */
/************************************************************************/
BOOL
CExceptEvent::DoBM(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{ 
    //bm addr a|w len
    assert(4 == argc);
    assert(pEvent != NULL);

    DWORD dwAddr = strtoul(&pszBuf[pargv[1]], NULL, 16);
    char  bpType  = pszBuf[pargv[2]];
    DWORD dwSize = strtoul(&pszBuf[pargv[3]], NULL, 16);
    assert(('a' == bpType) || ('w' == bpType));

    //check address validity
    CheckBMValidity(pEvent, dwAddr, 
                    (bpType == 'a') ? MEMBP_ACCESS : MEMBP_WRITE,
                    dwSize
                    );

    DoBML(pEvent, argc, pargv, pszBuf);
    DoBMPL(pEvent, argc, pargv, pszBuf);

    return TRUE;
}

BOOL
CExceptEvent::DoBML(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{  
    assert(pEvent != NULL);
    sprintf(g_szBuf, "----------------内存断点列表----------------\r\n"
                     "序号\t地址\t\t长度\t\t类型\r\n");

    list<tagMemBP>::iterator it;
    tagMemBP memBP;
    int i = 0;
    for (it = m_lstMemBP.begin(); 
         it != m_lstMemBP.end(); 
         it++, i++)
    {
        memBP = *it;
        _snprintf(g_szBuf, MAXBUF, "%s%d\t%p\t%p\t%s\r\n",
                                    g_szBuf,
                                    i,
                                    memBP.dwAddr, 
                                    memBP.dwSize,
                                    MEMBP_ACCESS == memBP.dwType ? "访问" : "写"
                                    );
    }
    pEvent->m_pUI->ShowInfo(g_szBuf);

    return TRUE;
}

BOOL 
CExceptEvent::DoBMPL(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    assert(pEvent != NULL);
    sprintf(g_szBuf, "----------------分页断点列表----------------\r\n");

    tagPageBP pageBP;
    tagMemBPInPage memBPInPage;
    map<DWORD, tagPageBP>::iterator it;
    list<tagMemBPInPage>::iterator itBPInPage;
    for (it = m_mapPage_PageBP.begin();
         it != m_mapPage_PageBP.end();
         it++)
    {
        pageBP = (*it).second;
        
        _snprintf(g_szBuf, MAXBUF, "%s分页地址\t旧属性\t\t新属性\r\n"
                                   "%p\t%p\t%p\r\n"
                                   "\t偏移\t长度\r\n",
                                    g_szBuf,
                                    pageBP.dwPageAddr,
                                    pageBP.dwOldProtect,
                                    pageBP.dwNewProtect);
        for (itBPInPage = pageBP.lstMemBP.begin();
             itBPInPage != pageBP.lstMemBP.end();
             itBPInPage++)
        {
            memBPInPage = *itBPInPage;
            _snprintf(g_szBuf, MAXBUF, "%s\t%04X\t%04X\r\n",
                                        g_szBuf,
                                        memBPInPage.wOffset,
                                        memBPInPage.wSize);
        }

    }

    pEvent->m_pUI->ShowInfo(g_szBuf);
    return TRUE;
}
