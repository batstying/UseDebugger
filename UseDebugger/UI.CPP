#include "UI.H"

//used for recording command history
static char gs_szCommand[MAXBUF];   

static HANDLE g_hLog = INVALID_HANDLE_VALUE;      //log for all operation
static HANDLE g_hTraceLog = INVALID_HANDLE_VALUE; //log for trace
//////////////////////////////////////////////////////////////////////////
CUI::CUI()
{
    m_pFileBuf = NULL;
    m_bImport  = FALSE;
    m_nFileSize = NULL;
}

BOOL CUI::m_bLog = FALSE;

CUI::~CUI()
{
    SafeClose(g_hLog);

    if (m_pFileBuf != NULL)
    {
        free(m_pFileBuf);
        m_pFileBuf = NULL;
    }
}

void
CUI::ShowMainMenu(void)
{
    printf("============UseDebugger============\r\n"
           "1. Debug New Process\r\n"
           "2. Debug Active Process\r\n"
           "3. Help\r\n"
           "0. Quit\r\n"
           "===================================\r\n"
           "Please Enter Your Choice By Number:\r\n");
}

/************************************************************************/
/* 
Function : get one char to determine user's choice
Param    : pch used to receive the input                                */
/************************************************************************/
void
CUI::GetCH(char *pch)
{
    scanf("%c", pch);
    fflush(stdin);
}

/************************************************************************/
/* 
Function : get input from user, and store them into pszBuf one by one
Params   : pargc used to know how many words
           pargv used to receive every start poisition within pszBuf
           pszBuf used to receive the input
              word1 NULL word2 NULL word3 NULL....*/
/************************************************************************/
void
CUI::GetInput(OUT int *pargc, 
              OUT int pargv[], 
              IN char *pszBuf, 
              IN SIZE_T nMaxBuf)
{
    char szBuf[MAXBYTE];
    int nRet;
    int nCount = 0;
    int nIndex = 0;
    char ch;
    static int nCurPos = 0; //for import script

    ShowInfo(">> ");
    while (TRUE)
    {  
        if (m_bImport)
        {
            nRet = sscanf(&m_pFileBuf[nCurPos], "%255s", szBuf);
        }
        else
        {
            nRet = scanf("%255s", szBuf);
        }

        if (EOF == nRet)
        {
            break;
        }

        if (nIndex + strlen(szBuf) + 1 > nMaxBuf)
        {
            break;
        }

        //used for recording into command history
        strcat(gs_szCommand, szBuf);
        strcat(gs_szCommand, " ");

        //command decode and save
        strcpy(&pszBuf[nIndex], szBuf);
        pargv[nCount] = nIndex;
        nIndex += (strlen(szBuf) + 1);
        nCount++;

        //
        if (m_bLog)
        {
            LogInfo(szBuf, g_hLog);
            LogInfo(" ", g_hLog);
        }

        //for import script
        if (m_bImport)
        {
            //
            printf("%s ", szBuf);
            nCurPos += (strlen(szBuf) + 1);
            if ('\n' == m_pFileBuf[nCurPos])
            {
                ch = '\n';
                nCurPos++;
                printf("\r\n");
            }

            //stop at es
            if (0 == strncmp(&m_pFileBuf[nCurPos], "es", 2))
            {
                m_bImport = FALSE;
            }
        }
        else
        {
            ch = getchar();
        }
        
        if('\n' == ch)
        {
            strcat(gs_szCommand, "\n");

            if (m_bLog)
            {
                LogInfo("\r\n", g_hLog);
            }

            break;
        }
    }

    *pargc = nCount;

    fflush(stdin);
}

/************************************************************************/
/* 
Function : select the file you want to debug, or save into or load from */
/************************************************************************/
BOOL CUI::SelectFile(char *pszFilePath, SIZE_T nMaxPath)
{
    BOOL bRet = TRUE;
    char appPath[MAX_PATH] = {0};	
    OPENFILENAME ofn;
    
    RtlZeroMemory(&ofn,sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    //ofn.lpstrFilter = TEXT("Executable Files\0*.exe;*.com;*.bat;*.pif\0All Files(*.*)\0*.*\0");
    ofn.lpstrFilter = TEXT("All Files(*.*)\0*.*\0");

    ofn.lpstrFile   = appPath;
    ofn.nMaxFile    = MAX_PATH;
    ofn.Flags       = OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES;
    
    bRet = GetOpenFileName(&ofn);	
    if (bRet)
    {
        lstrcpynA(pszFilePath, appPath, nMaxPath);
    }   

    return bRet;
}

static DWORD gs_dwReadWrite;
static BOOL gs_bRet;
/************************************************************************/
/* 
Function : log pszInfo into hFile                                                                     */
/************************************************************************/
void CUI::LogInfo(const char *pszInfo, HANDLE hFile)
{
    gs_bRet = WriteFile(hFile,
                        pszInfo,
                        strlen(pszInfo),
                        &gs_dwReadWrite,
                        NULL);
    if (!gs_bRet)
    {
        printf("Log error\r\n");
    }
}

void CUI::ShowInfo(const char *pszInfo)
{
    printf(pszInfo);
    
    if (m_bLog)
    {
        LogInfo(pszInfo, g_hLog);
    }    
}

void 
CUI::ShowErrorMessage()
{
    LPVOID lpMsgBuf;
    FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR) &lpMsgBuf,
        0,
        NULL 
        );
    
    ShowInfo((const char *)lpMsgBuf);
    ShowInfo("\r\n");
    LocalFree(lpMsgBuf);
}

/************************************************************************/
/* 
Function : export the user's command history                           */
/************************************************************************/
void CUI::ExportScript()
{
    BOOL bRet;
    DWORD dwWrite;
    char szFilePath[MAX_PATH];
    bRet = SelectFile(szFilePath, MAX_PATH);
    if (!bRet)
    {
        ShowErrorMessage();
        return;
    }

    HANDLE hFile = CreateFile(szFilePath,
                            GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );
    
    if (INVALID_HANDLE_VALUE == hFile)
    {
        ShowErrorMessage();
        return;
    }

    bRet = WriteFile(hFile, 
                  gs_szCommand,
                  strlen(gs_szCommand),
                  &dwWrite,
                  NULL);
    if (!bRet)
    {
        ShowErrorMessage();
    }

    if (hFile != NULL)  //what about invalid
    {
        CloseHandle(hFile);
        hFile = NULL;
    }
}

/************************************************************************/
/* 
Function : load and execute script saved before                                                                     */
/************************************************************************/
void CUI::ImportScript()
{
    BOOL bRet;
    char szFilePath[MAX_PATH];
    bRet = SelectFile(szFilePath, MAX_PATH);
    if (!bRet)
    {
        ShowErrorMessage();
        return;
    }

    //load file into memory
    if (!LoadFile(szFilePath, &m_pFileBuf, &m_nFileSize))
    {
        return;
    }    

    m_bImport = TRUE;
}

/************************************************************************/
/* 
Function : enable the run-time logging for everything
          the user input, the output, whatever you can see on screen                                                                     */
/************************************************************************/
void CUI::Log()
{   
    printf("Please Select the File You want to Save the log:\r\n");

    char szFilePath[MAX_PATH];
    BOOL bRet = SelectFile(szFilePath, MAX_PATH);
    if (!bRet)
    {
        ShowErrorMessage();
        return;
    }

    g_hLog = CreateFile(szFilePath,
                        FILE_APPEND_DATA,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    
    if (INVALID_HANDLE_VALUE == g_hLog)
    {
        ShowErrorMessage();
        return;
    }

    m_bLog = TRUE;
}

/************************************************************************/
/* 
Function : prepare file for trace log                                    */
/************************************************************************/
void
CUI::PreTrace()
{
    printf("Please Select the File You want to Save the trace log:\r\n");
    
    char szFilePath[MAX_PATH];
    BOOL bRet = SelectFile(szFilePath, MAX_PATH);
    if (!bRet)
    {
        ShowErrorMessage();
        return;
    }
    
    g_hTraceLog = CreateFile(szFilePath,
                        FILE_APPEND_DATA,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );
    
    if (INVALID_HANDLE_VALUE == g_hTraceLog)
    {
        ShowErrorMessage();
        return;
    }
}

/************************************************************************/
/* 
Function : write trace log into file                                                                     */
/************************************************************************/
void
CUI::TraceLog(const char *pszInfo)
{
    LogInfo(pszInfo, g_hTraceLog);
}