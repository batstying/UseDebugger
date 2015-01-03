// BaseEvent.cpp: implementation of the CBaseEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseEvent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseEvent::CBaseEvent()
{
    m_hProcess = NULL;
    m_hThread = NULL;
    ZeroMemory(&m_debugEvent, sizeof(DEBUG_EVENT));
    ZeroMemory(&m_Context, sizeof(CONTEXT));

    m_pUI = NULL;
    m_pUI = new CUI();
    assert(m_pUI != NULL);

    m_bTalk = FALSE;

    m_dwAddr = NULL;
    m_dwFS   = NULL;

    m_bAccessVioTF = FALSE;
    m_bNormalBPTF = FALSE;
    m_bUserTF     = FALSE;
    m_bHWBPTF     = FALSE;
    m_bStepOverTF = FALSE;

    m_bTmpBP = FALSE;
}

CBaseEvent::~CBaseEvent()
{
    if (m_pUI != NULL)
    {
        delete m_pUI;
        m_pUI = NULL;
    }

    SafeClose(m_hProcess);
    SafeClose(m_hThread);
}

//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/* 
Function : judge whether is call instructions pointed by eip
Params   : pnLen used to receive the instruction size 
Return   : TRUE if is call, FALSE otherwise

004012B6  |.  FF15 A8514200 CALL DWORD PTR DS:[<&KERNEL32.GetVersion>;  kernel32.GetVersion
0040130E  |.  E8 9D2A0000   CALL testDbg.00403DB0                    ; \testDbg.00403DB0
  
*/
/************************************************************************/
BOOL
CBaseEvent::IsCall(DWORD *pnLen)
{
    assert(pnLen != NULL);

    static char szCodeBuf[64];
    static char szOpcode[64];
    static char szASM[128];
    UINT nCodeSize;
     
    //not a good idea to use EIP as default, not universal...., but makes the caller easier
    BOOL bRet = ReadBuf(m_hProcess, (LPVOID)m_Context.Eip, szCodeBuf, sizeof(szCodeBuf));    
    if (!bRet)
    {
        return FALSE;
    }
    
    Decode2AsmOpcode((PBYTE)szCodeBuf,
                    szASM,
                    szOpcode, 
                    &nCodeSize,
                    m_Context.Eip);

    *pnLen = nCodeSize;

    if (0 == memcmp(szOpcode, "E8", 2)
        || 0 == memcmp(szOpcode, "FF15", 4)
        //others
        )
    {
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
BOOL 
CBaseEvent::ReadBuf(HANDLE hProcess, LPVOID lpAddr, LPVOID lpBuf, SIZE_T nSize)
{
    BOOL bRet = ReadProcessMemory(
                        hProcess, 
                        lpAddr,
                        lpBuf,
                        nSize,
                        NULL);
    
    if (!bRet)
    {
        CUI::ShowErrorMessage();
        return FALSE;
    }

    return TRUE;
}
