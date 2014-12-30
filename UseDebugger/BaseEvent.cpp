// BaseEvent.cpp: implementation of the CBaseEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "BaseEvent.h"

static char gs_szCodeBuf[64];
static char gs_szOpcode[64];
static char gs_szASM[128];

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

    m_bAccessVioTF = FALSE;
    m_bNormalBPTF = FALSE;
    m_bUserTF     = FALSE;

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
//Registers related
//see IA1.pdf 3.4.3 EFLAG registers
typedef struct _tagEFlags
{
    unsigned char CF:1;
    unsigned char Reserv1:1; //1
    unsigned char PF:1;
    unsigned char Reserv2:1; //0
    unsigned char AF:1; 
    unsigned char Reserv3:1; //0
    unsigned char ZF:1;
    unsigned char SF:1;
    unsigned char TF:1;
    unsigned char IF:1;
    unsigned char DF:1;
    unsigned char OF:1;
    //others
    unsigned char IOPL:2;
    unsigned char NT:1;
    unsigned char Reserv4:1; //0
    unsigned char Remain;
}tagEFlags;

void 
CBaseEvent::DoShowRegs()
{
    tagEFlags eflg = *(tagEFlags *)&m_Context.EFlags;
    _snprintf(g_szBuf, MAXBUF, "EAX=%08X ECX=%08X EDX=%08X EBX=%08X\r\n"
                                "ESP=%08X EBP=%08X ESI=%08X EDI=%08X\r\n"
                                "EIP=%08X CS=%04X DS=%04X ES=%04X SS=%04X FS=%04X\r\n"
                                "OF=%1X DF=%1X IF=%1X SF=%1X ZF=%1X AF=%1X PF=%1X CF=%1X\r\n",
                                m_Context.Eax, m_Context.Ecx, m_Context.Edx, m_Context.Ebx,
                                m_Context.Esp, m_Context.Ebp, m_Context.Esi, m_Context.Edi,
                                m_Context.Eip, 
                                m_Context.SegCs, m_Context.SegDs, m_Context.SegEs,
                                m_Context.SegSs, m_Context.SegFs,
                                eflg.OF, eflg.DF, eflg.IF, eflg.SF, 
                                eflg.ZF, eflg.AF, eflg.PF, eflg.CF);
    
    m_pUI->ShowInfo(g_szBuf);  
    
    this->ShowTwoASM();
}

//prototype
extern "C"
void
__stdcall
Decode2AsmOpcode(IN PBYTE pCodeEntry,   // 需要解析指令地址
                 OUT char* strAsmCode,        // 得到反汇编指令信息
                 OUT char* strOpcode,         // 解析机器码信息
                 OUT UINT* pnCodeSize,        // 解析指令长度
                 IN UINT nAddress);
void 
CBaseEvent::ShowOneASM()
{
    UINT nCodeSize;
    BOOL bRet;
    
    bRet = ReadProcessMemory(
                        m_hProcess, 
                        (LPVOID)(m_Context.Eip),
                        gs_szCodeBuf,
                        sizeof(gs_szCodeBuf),
                        NULL);
    
    if (!bRet)
    {
        CUI::ShowErrorMessage();
    }
    
    Decode2AsmOpcode((PBYTE)gs_szCodeBuf,
                    gs_szASM,
                    gs_szOpcode, 
                    &nCodeSize,
                    m_Context.Eip);
    
    _snprintf(g_szBuf, MAXBUF, "%p:  %-16s   %-16s   [%d]\r\n",
                                m_Context.Eip, 
                                gs_szOpcode, 
                                gs_szASM, 
                                nCodeSize);
    m_pUI->ShowInfo(g_szBuf);
}
void 
CBaseEvent::ShowTwoASM()
{
    UINT nCodeSize;
    UINT nCodeSize1;
    BOOL bRet;
    
    bRet = ReadProcessMemory(
                    m_hProcess, 
                    (LPVOID)(m_Context.Eip),
                    gs_szCodeBuf,
                    sizeof(gs_szCodeBuf),
                    NULL);
    
    if (!bRet)
    {
        CUI::ShowErrorMessage();
    }
    
    Decode2AsmOpcode((PBYTE)gs_szCodeBuf,
                    gs_szASM,
                    gs_szOpcode, 
                    &nCodeSize,
                    m_Context.Eip);
    
    _snprintf(g_szBuf, MAXBUF, "%p:  %-16s   %-16s   [%d]\r\n",
                                m_Context.Eip, 
                                gs_szOpcode, 
                                gs_szASM, 
                                nCodeSize);
    m_pUI->ShowInfo(g_szBuf);
    
    Decode2AsmOpcode((PBYTE)gs_szCodeBuf + nCodeSize,
                        gs_szASM,
                        gs_szOpcode, 
                        &nCodeSize1,
                        m_Context.Eip + nCodeSize);
    
    _snprintf(g_szBuf, MAXBUF, "%p:  %-16s   %-16s   [%d]\r\n",
            m_Context.Eip + nCodeSize, 
            gs_szOpcode, 
            gs_szASM, 
            nCodeSize1);
    m_pUI->ShowInfo(g_szBuf);        
}

BOOL
CBaseEvent::DoShowASM(int argc, int pargv[], const char *pszBuf)
{ 
    //u
    return TRUE;
}

BOOL
CBaseEvent::DoShowData(int argc, int pargv[], const char *pszBuf)
{ 
    //d
    return TRUE;
}
