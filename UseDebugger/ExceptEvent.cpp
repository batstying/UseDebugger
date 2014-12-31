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
Function : Check whether hit the MemBP
Params   : dwAddr is the addr to be checked
           ppageBP contains the PageBP info where the dwAddr in
Process:
                                             */
/************************************************************************/
BOOL 
CheckHitMemBP(CBaseEvent *pEvent, DWORD dwAddr, tagPageBP *ppageBP)
{
    assert(ppageBP != NULL);
    DWORD dwOffset = dwAddr - ppageBP->dwPageAddr;

    tagMemBPInPage *pmemBPInPage = NULL;
    list<tagMemBPInPage>::iterator it;
    for (it = ppageBP->lstMemBP.begin();
         it != ppageBP->lstMemBP.end();
         it++)
    {
        pmemBPInPage = &(*it);
        if (dwOffset >= pmemBPInPage->wOffset
            && dwOffset < pmemBPInPage->wOffset + pmemBPInPage->wSize)
        {
            //hit
            return TRUE;
        }
    }
         
    return FALSE;
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
            CUI::ShowErrorMessage();
            return DBG_CONTINUE;    //really?
        }

        //pEvent->ShowOneASM();

        //now judge whether hit the MemBP
        bRet = CheckHitMemBP(pEvent, dwAddr, ppageBP);
        if (bRet)
        {
            pEvent->DoShowRegs();
            _snprintf(g_szBuf, MAXBUF, "Hit MemBP %p %s\r\n",
                                     dwAddr,
                                     0 == exceptRecord.ExceptionInformation[0] ? "read" : "write"
                                     );
            pEvent->m_pUI->ShowInfo(g_szBuf);
            pEvent->m_bTalk = TRUE;
        }

        //need to set single step to restore the protect
        m_bAccessVioTF = TRUE;
        m_dwAddr       = dwAddr;
        DoStepInto(pEvent/*, 1, argv, g_szBuf*/);
        return DBG_CONTINUE;
    }

    return dwContinueStatus;
}

DWORD
CExceptEvent::OnBreakPoint(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
    
    //system breakpoint
    static BOOL bSysPoint = TRUE;
    if (bSysPoint)
    {
        pEvent->m_bTalk = TRUE;
        pEvent->DoShowRegs();

        bSysPoint = FALSE;
        return DBG_CONTINUE;
    }

    EXCEPTION_DEBUG_INFO exceptInfo = pEvent->m_debugEvent.u.Exception;
    EXCEPTION_RECORD exceptRecord = exceptInfo.ExceptionRecord;
    DWORD dwAddr = (DWORD)exceptRecord.ExceptionAddress;

    //whether been set normalBP
    tagNormalBP *pNormalBP = NULL;
    BOOL bRet = HasNormalBP(pEvent, dwAddr, &pNormalBP);
    if (bRet)
    {
        assert(pNormalBP != NULL);
    
        //now restore the code
        bRet = WriteProcessMemory(pEvent->m_hProcess,
                                (LPVOID)dwAddr,
                                (LPVOID)&pNormalBP->oldvalue,
                                sizeof(gs_BP),
                                NULL);
        if (!bRet)
        {
            CUI::ShowErrorMessage();
        }

        if (pNormalBP->bPerment)
        {
            //need to set single step
            m_bNormalBPTF = TRUE;
            m_dwAddr = dwAddr;
            DoStepInto(pEvent);
        }

        pEvent->m_Context.Eip--;
        pEvent->m_bTalk = TRUE;
        pEvent->DoShowRegs();

        return DBG_CONTINUE;
    }
    
    return dwContinueStatus;
}

/************************************************************************/
/* 
Function : Check whether hit the Hardware Breakpoint
Params   : pEvent
Return   : TRUE if yes, FALSE otherwise 
Process  : check DR6
                                                                               */
/************************************************************************/
BOOL
CExceptEvent::HasHitHWBP(CBaseEvent *pEvent)
{
    DWORD dwIndex = 0;
    dwIndex = (pEvent->m_Context.Dr6 & 0x0F);
    if (0 == dwIndex)
    {
        return FALSE;
    }

    for (int i = 0; dwIndex != 1; i++)
    {
        dwIndex >>= 1;
    }

    dwIndex = i;
    
    tagHWBP hwBP;
    hwBP.pDRAddr[0] = &pEvent->m_Context.Dr0;
    hwBP.pDRAddr[1] = &pEvent->m_Context.Dr0;
    hwBP.pDRAddr[2] = &pEvent->m_Context.Dr0;
    hwBP.pDRAddr[3] = &pEvent->m_Context.Dr0;
    
    DWORD dwDR7 = pEvent->m_Context.Dr7;
    DWORD dwLENRW = dwDR7 >> 16;
    for (i = 0 ; i < dwIndex; i++)
    {
        dwLENRW >>= 4;
    }

    hwBP.dwAddr = *(hwBP.pDRAddr[dwIndex]);
    hwBP.dwType = dwLENRW & 0x3;
    dwLENRW >>= 2;
    hwBP.dwLen = (dwLENRW & 0x03) + 1;
    dwLENRW >>= 2;

    pEvent->DoShowRegs();
    sprintf(g_szBuf, "HWBP Hit: %p\t%d\t%s *****************\r\n",
                                hwBP.dwAddr,
                                hwBP.dwLen,
                                (HWBP_EXECUTE == hwBP.dwType) ? STREXECUTE : 
                                ((HWBP_WRITE == hwBP.dwType) ? STRWRITE: STRACCESS)
            );
    pEvent->m_pUI->ShowInfo(g_szBuf);
    
    return TRUE;
}

DWORD
CExceptEvent::OnSingleStep(CBaseEvent *pEvent)
{
    assert(pEvent != NULL);
    DWORD dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;

    //////////////////////////////////////////////////////////////////////////
    //check hardware
    if (HasHitHWBP(pEvent))
    {
        pEvent->m_bTalk = TRUE;
        return DBG_CONTINUE;
    }
    
    //////////////////////////////////////////////////////////////////////////
    //AccessViolation
    tagPageBP *ppageBP = NULL;
    DWORD dwOldProtect;
    BOOL bRet;
    if (m_bAccessVioTF)
    {
        m_bAccessVioTF = FALSE;
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
                CUI::ShowErrorMessage();
                return DBG_CONTINUE;    //really?
            }
        }
        return DBG_CONTINUE;
    }

    //////////////////////////////////////////////////////////////////////////
    //NormalBP
    tagNormalBP *pNomalBP = NULL;
    if (m_bNormalBPTF)
    {
        m_bNormalBPTF = FALSE;

        bRet = HasNormalBP(pEvent, m_dwAddr, &pNomalBP);
        if (!bRet)
        {
            return DBG_CONTINUE;
        }

        //restore the code
        assert(pNomalBP->bPerment);
        bRet = WriteProcessMemory(pEvent->m_hProcess,
                                (LPVOID)m_dwAddr,
                                (LPVOID)&g_szBuf,
                                sizeof(gs_BP),
                                NULL);
        if (!bRet)
        {
            CUI::ShowErrorMessage();
        }

        if (pNomalBP->bPerment)
        {
            pEvent->DoShowRegs();
        }

        return DBG_CONTINUE;
    }

    //////////////////////////////////////////////////////////////////////////
    //now for user input 't'
    if (pEvent->m_bUserTF)
    {
        pEvent->m_bUserTF = FALSE;
        pEvent->m_bTalk = TRUE;
        pEvent->DoShowRegs();
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
CExceptEvent::DoStepInto(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/)
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
    assert(ppPageBP != NULL);
    *ppPageBP = NULL;

    DWORD dwPageAddr = (dwAddr / m_dwPageSize) * m_dwPageSize;
    map<DWORD, tagPageBP>::iterator it;
    it = m_mapPage_PageBP.find(dwPageAddr);
    if (it != m_mapPage_PageBP.end())
    {
        *ppPageBP = &it->second;
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/* 
Function : judge whether the addr HAS NormalBP RECORD
           BUT IT'S THE USER'S DUTY to dertermine 
           1) whether it's tmp or permanent, 
           //2) if permanent,  whether, enabled or disabled.
Params   : dwAddr is the specified address
            [OUT] ppNormalBP used to receive the normalBP info (if exists)
Return   : TRUE if already been recorded.
            FALSE otherwise              
Alert    : Again, IT'S THE USER'S DUTY to dertermine 
            1) whether it's tmp or permanent, 
            //2) if permanent,  whether, enabled or disabled*/
/************************************************************************/
BOOL
CExceptEvent::HasNormalBP(CBaseEvent *pEvent, DWORD dwAddr, tagNormalBP **ppNormalBP)
{
    assert(pEvent != NULL);
    assert(ppNormalBP != NULL);
    *ppNormalBP = NULL;

    map<DWORD, tagNormalBP>::iterator it;
    it = m_mapAddr_NormBP.find(dwAddr);
    if (it != m_mapAddr_NormBP.end())
    {
        *ppNormalBP = &it->second;
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
    assert((dwAddr != 0) && (dwAddr != ULONG_MAX));

    //whether has the record, (but not sure tmp or permanent, //enabled or disabled)
    tagNormalBP *pNormalBP = NULL;
    bRet = HasNormalBP(pEvent, dwAddr, &pNormalBP);
    if (bRet)
    {
        assert(pNormalBP != NULL);

        //if to set tmp
        if (pEvent->m_bTmpBP)
        {
            if (pNormalBP->bTmp)
            {
                //
            }
            else if (pNormalBP->bPerment)
            {
                pNormalBP->bTmp = TRUE;
            }
            else
            {
                //both not tmp and permanent, no possible
                assert(FALSE);
            }
            pEvent->m_bTmpBP = FALSE;
        }
        else    
        {
            if (pNormalBP->bTmp)
            {
                pNormalBP->bPerment = TRUE;
            }
            else if (pNormalBP->bPerment)
            {
                //
            }
            else
            {
                //not possible
                assert(FALSE);
            }
        }
        return TRUE;
    }

    //////////////////////////////////////////////////////////////////////////
    //Now has no record about the normalbp

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
            CUI::ShowErrorMessage();
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
        CUI::ShowErrorMessage();
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
        CUI::ShowErrorMessage();
        return FALSE;
    }

    //now save the NormalBP

    if (pEvent->m_bTmpBP)
    {
        normalBP.bTmp = TRUE;
        pEvent->m_bTmpBP = FALSE;
    }
    else
    {
        normalBP.bPerment = TRUE;
    }
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
    DWORD dwRealSize = 0;
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
                dwPageAddr += m_dwPageSize;
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
            dwPageAddr += m_dwPageSize;
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
                            (LPVOID)dwPageAddr,
                            1,
                            PAGE_NOACCESS,
                            &dwOldProtect
                             );
        if (!bRet)
        {
            CUI::ShowErrorMessage();
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
    assert((dwAddr != 0) && (dwAddr != ULONG_MAX));

    char  bpType  = pszBuf[pargv[2]];
    DWORD dwSize = strtoul(&pszBuf[pargv[3]], NULL, 16);
    assert((dwSize != 0) && (dwSize != ULONG_MAX));
    assert(('a' == bpType) || ('w' == bpType));

    //check address validity
    CheckBMValidity(pEvent, dwAddr, 
                    (bpType == 'a') ? MEMBP_ACCESS : MEMBP_WRITE,
                    dwSize
                    );

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

/************************************************************************/
/* 
Function : judge whether has other memBP within the page
Params   : dwPageAddr indicate the page
           ppPageBP used to receive the PageBP info
Return   : TRUE if has other memBPS, Otherwise FALSE                                                                    */
/************************************************************************/
BOOL
CExceptEvent::HasOtherMemBP(CBaseEvent *pEvent, DWORD dwPageAddr, tagPageBP **ppPageBP)
{
    assert(ppPageBP != NULL);
    *ppPageBP = &m_mapPage_PageBP[dwPageAddr];

    list<tagMemBPInPage> &lstmemBP = m_mapPage_PageBP[dwPageAddr].lstMemBP;
    list<tagMemBPInPage>::iterator it;
    DWORD i = 0;
    for (it = lstmemBP.begin();
        it != lstmemBP.end();
        it++, i++)
    {
        if (i > 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/************************************************************************/
/* 
Function : Remove the specified memory breakpoint
Params   : bmc id
           id is the index shown in bml

Process  :
                               */
/************************************************************************/
BOOL
CExceptEvent::DoBMC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    //bmc id
    assert(pEvent != NULL);
    assert(pszBuf != NULL);
    assert(2 == argc);

    DWORD i = 0;
    DWORD j = strtoul(&pszBuf[pargv[1]], NULL, 10);  //not a good idea
    DWORD dwAddr = 0;
    DWORD dwSize = 0;
    DWORD dwType = 0;

    //get dwAddr, dwSize, dwType
    list<tagMemBP>::iterator itMemBP;
    for (itMemBP = m_lstMemBP.begin();
        itMemBP != m_lstMemBP.end();
        itMemBP++, i++)
    {
        if (j == i)
        {
            dwAddr = (*itMemBP).dwAddr;
            dwSize = (*itMemBP).dwSize;
            dwType = (*itMemBP).dwType;
            
            //remove from MemBP
            m_lstMemBP.remove(*itMemBP);
            break;
        }
    }

    //if no match
    if (0 == dwAddr)
    {
        return FALSE;
    }

    //how many pages may be involed
    DWORD nPages = (dwAddr + dwSize) / m_dwPageSize - dwAddr / m_dwPageSize + 1;
    
    //check these pages state 
    MEMORY_BASIC_INFORMATION memInfo;
    tagMemBPInPage memBPInPage;         //断点在分页内信息
    tagPageBP      *ppageBP = NULL;
    map<DWORD, tagPageBP>::iterator it;
    map<DWORD, tagPageBP>::iterator itend = m_mapPage_PageBP.end();
    DWORD  dwPageAddr = (dwAddr / m_dwPageSize) * m_dwPageSize;
    DWORD dwOldProtect;
    BOOL bRet;

    //whether this page contains other membp
    for (i = 0; i < nPages; i++)
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

        //if protect already not set
#if 0
        if (PAGE_NOACCESS != memInfo.Protect)
        {
            dwPageAddr += m_dwPageSize;
            continue;
        }
#endif
        
        //if no need to set
        if ((MEMBP_WRITE == dwType)
            && (PAGE_READONLY == dwType
            || PAGE_EXECUTE == dwType
            || PAGE_EXECUTE_READ == dwType)
            //others?
            )
        {
            dwPageAddr += m_dwPageSize;
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

        //if has no other memBP within the page, now can restore the protect
        if (!HasOtherMemBP(pEvent, dwPageAddr, &ppageBP))
        {
            bRet = VirtualProtectEx(pEvent->m_hProcess,
                                (LPVOID)dwPageAddr,
                                1,
                                ppageBP->dwOldProtect,
                                &dwOldProtect
                                );
            if (!bRet)
            {
                CUI::ShowErrorMessage();
            }
        }
        
        //remove from PageBP info
        m_mapPage_PageBP[dwPageAddr].lstMemBP.remove(memBPInPage);
        
        dwPageAddr += m_dwPageSize;        
    } 

    return TRUE;
}

/************************************************************************/
/* 
Function : Set Hardware Break Point for the specified addr and len 
Params   :  pHWBP contains the HWBP info by user

Process  :
          0) whether the addr valid
          1) whether DR0 ~ DR3 available
          2) fix the align 
          */
/************************************************************************/
BOOL
CExceptEvent::SetHWBP(CBaseEvent *pEvent, tagHWBP *pHWBP)
{
    assert(pEvent != NULL);
    assert(pHWBP != NULL);

    //whether the page valid
    BOOL bRet = IsPageValid(pEvent, pHWBP->dwAddr);
    if (!bRet)
    {
        return FALSE;
    }

    //fix align
    if (0x01 == (pHWBP->dwAddr & 0x01))
    {
        pHWBP->dwLen = 1;
    }
    else if ((0x2 == (pHWBP->dwAddr & 0x2))
            && (0x4 == pHWBP->dwLen)
            )
    {
        pHWBP->dwLen = 2;
    }

    DWORD dwLen = pHWBP->dwLen - 1;  //00 ->1byte, 01 -> 2byte, 11 -> 3byte

    //
    tagDR7 *pdr7 = (tagDR7 *)(&pEvent->m_Context.Dr7);
    pEvent->m_Context.Dr7 |= DR7INIT;
    DWORD dwDR7 = pEvent->m_Context.Dr7;
    DWORD dwCheck = 0x03;
    DWORD dwSet   = 0x01;
    DWORD dwLENRW = (((dwLen << 2) | pHWBP->dwType) << 16);
    int i = 0;
    for ( ; i < 4; i++)
    {
        //if both GX, LX is zero, then DRX is available
        //or the same addr
        if (0 == (dwDR7 & dwCheck)
            || (*(pHWBP->pDRAddr[i]) ==pHWBP->dwAddr))
        {
            *(pHWBP->pDRAddr[i])  = pHWBP->dwAddr;    //DR0 = dwAddr   
            pEvent->m_Context.Dr7 |= dwSet;                          //pdr7->GL0 = 1;
            pEvent->m_Context.Dr7 |= dwLENRW;
            break;
        }

        dwCheck <<= 2;
        dwSet   <<= 2;
        dwLENRW <<= 4;
    }

    //no availabe
    if (4 == i)
    {
        return FALSE;
    }

    return TRUE;

#if 0
    //find the available DR0~DR3, can be more beautiful
    DWORD *pDRX = NULL;
    int nFree = -1;
    tagDR7 *pdr7 = (tagDR7 *)(&pEvent->m_Context.Dr7);
    if (0 == pdr7->GL0)
    {
        nFree = 0;
        pDRX  = &pEvent->m_Context.Dr0;
        pdr7->GL0 = 1;
        pdr7->LEN0 = dwLen;
        pdr7->RW0  = pHWBP->dwType;
    }
    else if (0 == pdr7->GL1)
    {
        nFree = 1;
        pDRX  = &pEvent->m_Context.Dr1;
        pdr7->GL1 = 1;
        pdr7->LEN1 = dwLen;
        pdr7->RW1  = pHWBP->dwType;
    }
    else if (0 == pdr7->GL2)
    {
        nFree = 2;
        pDRX  = &pEvent->m_Context.Dr2;
        pdr7->GL2 = 1;
        pdr7->LEN2 = dwLen;
        pdr7->RW2  = pHWBP->dwType;
    }
    else if (0 == pdr7->GL3)
    {
        nFree = 3;
        pDRX  = &pEvent->m_Context.Dr3;
        pdr7->GL3 = 1;
        pdr7->LEN3 = dwLen;
        pdr7->RW3  = pHWBP->dwType;
    }
    
    if (-1 == nFree)
    {
        return FALSE;
    }

    return TRUE;
#endif 
}

BOOL 
CExceptEvent::DoBH(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    //bh addr e|w|a 1|2|4
    assert(pEvent != NULL);
    assert(pszBuf != NULL);
    assert(4 == argc);
   
    DWORD dwAddr = strtoul(&pszBuf[pargv[1]], NULL, 16);
    assert((dwAddr != 0) && (dwAddr != ULONG_MAX));

    char chType  = pszBuf[pargv[2]];
    assert('e' == chType || 'w' == chType || 'a' == chType);
    DWORD dwType = ('e' == chType) ? HWBP_EXECUTE : 
                    (('w' == chType) ? HWBP_WRITE : HWBP_ACCESS );

    char chLen   = pszBuf[pargv[3]];
    assert('1' == chLen || '2' == chLen || '4' == chLen);
    DWORD dwLen = strtoul(&chLen, NULL, 10);

    //can be more beautiful, constructor
    tagHWBP hwBP;
    hwBP.dwAddr = dwAddr;
    hwBP.dwType = dwType;
    hwBP.dwLen  = dwLen;
    hwBP.pDRAddr[0] = &pEvent->m_Context.Dr0;
    hwBP.pDRAddr[1] = &pEvent->m_Context.Dr1;
    hwBP.pDRAddr[2] = &pEvent->m_Context.Dr2;
    hwBP.pDRAddr[3] = &pEvent->m_Context.Dr3;

    SetHWBP(pEvent, &hwBP);

    return TRUE;
}

BOOL 
CExceptEvent::DoBHL(CBaseEvent *pEvent/*, int argc, int pargv[], const char *pszBuf*/)
{
    tagDR7 *pdr7 = (tagDR7 *)(&pEvent->m_Context.Dr7);
    DWORD dwDR7 = pEvent->m_Context.Dr7;
    DWORD dwCheck = 0x03;
    DWORD dwLENRW = dwDR7 >> 16;
    tagHWBP hwBP;
    hwBP.pDRAddr[0] = &pEvent->m_Context.Dr0;   //can be more beautiful
    hwBP.pDRAddr[1] = &pEvent->m_Context.Dr1;
    hwBP.pDRAddr[2] = &pEvent->m_Context.Dr2;
    hwBP.pDRAddr[3] = &pEvent->m_Context.Dr3;

    sprintf(g_szBuf, "----------------硬件断点列表----------------\r\n"
                     "序号\t地址\t\t长度\t类型\r\n");
    int i = 0;
    for ( ; i < 4; i++)
    {
        //if both GX, LX is zero, then DRX is not set
        if (0 == (dwDR7 & dwCheck))
        {
            dwCheck <<= 2;
            dwLENRW >>= 4;
            continue;
        }
       
        dwCheck <<= 2;

        hwBP.dwAddr = *(hwBP.pDRAddr[i]);
        hwBP.dwType = dwLENRW & 0x3;
        dwLENRW >>= 2;
        hwBP.dwLen = (dwLENRW & 0x03) + 1;
        dwLENRW >>= 2;

        _snprintf(g_szBuf, MAXBUF, "%s%d\t%p\t%d\t%s\r\n",
                                    g_szBuf,
                                    i,
                                    hwBP.dwAddr,
                                    hwBP.dwLen,
                                    (HWBP_EXECUTE == hwBP.dwType) ? STREXECUTE : 
                                    ((HWBP_WRITE == hwBP.dwType) ? STRWRITE: STRACCESS)
                                    );
    }

    pEvent->m_pUI->ShowInfo(g_szBuf);

    return TRUE;
}  

/************************************************************************/
/* 
Function : remove the specified HWBP                                   */
/************************************************************************/
BOOL 
CExceptEvent::DoBHC(CBaseEvent *pEvent, int argc, int pargv[], const char *pszBuf)
{
    //bhc id
    assert(pEvent != NULL);
    assert(pszBuf != NULL);

    DWORD dwIndex = strtoul(&pszBuf[pargv[1]], NULL, 10);
    assert(dwIndex <= 4);

    DWORD dwSet = 0x3;
    for (DWORD i = 0; i < dwIndex; i++)
    {
        dwSet <<= 2;
    }

    pEvent->m_Context.Dr7 &= (~dwSet);
    
    return TRUE;
}