#include "UI.H"

void
CUI::ShowMainMenu(void)
{
    printf("============UseDebugger============\r\n"
           "1. Debug New Process\r\n"
           "2. Debug Active Process\r\n"
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
CUI::GetInput(int *pargc, int pargv[], char *pszBuf, SIZE_T nMaxBuf)
{
    char szBuf[MAXBYTE];
    int nRet;
    int nCount = 0;
    int nIndex = 0;
    char ch;

    printf(">> ");
    while (TRUE)
    {
        nRet = scanf("%255s", szBuf);
        if (EOF == nRet || '#' == szBuf[0])
        {
            break;
        }

        if (nIndex + strlen(szBuf) + 1 > nMaxBuf)
        {
            break;
        }

        strcpy(&pszBuf[nIndex], szBuf);
        pargv[nCount] = nIndex;
        nIndex += (strlen(szBuf) + 1);
        nCount++;

        ch = getchar();
        if('\n' == ch)
        {
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
    ofn.lpstrFilter = TEXT("Executable Files\0*.exe;*.com;*.bat;*.pif\0All Files(*.*)\0*.*\0");
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

void 
CUI::ShowInfo(const char *pszInfo)
{
    printf(pszInfo);
}