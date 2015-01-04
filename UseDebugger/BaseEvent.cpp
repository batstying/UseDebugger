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
    m_bTrace = FALSE;
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
