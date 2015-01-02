#include "Common.h"

//////////////////////////////////////////////////////////////////////////
#define SUCCESS     0x00
#define NOT_EXIST   0x01
#define BAD_PARAM   0x02
#define FAILURE      0x03    //other reason

#define MAX_LINE    1024

static FILE *gs_fpLog;

static long gs_lFileSize;   
static char *gs_pszFile;    //file content
static char gs_pszLine[MAX_LINE];

typedef struct _tagStatInfo
{
    int m_nBlankLine;
    int m_nCommentLine;
    int m_nTotalLine;
    map<string, int> m_map_word_count;  //word:count
}tagStatInfo;

static tagStatInfo gs_tagRet;   //统计结果

static void
Usage(void)
{
    printf("Please use like this:   IDA_Tool.exe ida.txt\r\n");
}

static void
LogInfo(char *pszInfo)
{
    printf("%s\r\n", pszInfo);
}

static void
SkipSpace(char **pszline)
{
    while (' ' == **pszline)
    {
        (*pszline)++;
    }
}

/************************************************************************/
/* Try to load file into memory,
   return SUCCESS if okay
   return BAD_PARAM if pszFileName is NULL
   return NOT_EXIST if open file error
   return FAILED for other reason

*/
/************************************************************************/
int
LoadFile(char *pszFileName)
{
    FILE *fp      = NULL;
    int nRet      = SUCCESS;
    char *pszInfo = "LoadFile Success";

    if (NULL == pszFileName)
    {
        nRet    = BAD_PARAM;
        pszInfo = "Bad Param For File Name Provided";
        goto END;
    }

    fp = fopen(pszFileName, "rb");
    if (NULL == fp)
    {
        nRet    = NOT_EXIST;
        pszInfo = "Open the File Failed";
        goto END;
    }

    //get file size
    nRet = fseek(fp, 0, SEEK_END);
    if (nRet)
    {
        nRet    = FAILURE;
        pszInfo = "fseek to the end of file failed";
        goto END;
    }

    gs_lFileSize = ftell(fp);
    if (-1L == gs_lFileSize)
    {
        nRet    = FAILURE;
        pszInfo = "ftell failed";
        goto END;
    }

    //rollback
    nRet = fseek(fp, 0, SEEK_SET);
    if (nRet)
    {
        nRet    = FAILURE;
        pszInfo = "fseek to the beginning of file failed";
        goto END;
    }

    //alloc mem and load file into
    gs_pszFile = (char *)malloc(gs_lFileSize);
    if (NULL == gs_pszFile)
    {
        nRet    = FAILURE;
        pszInfo = "malloc of file size failed";
        goto END;
    }

    fread(gs_pszFile, sizeof(char), gs_lFileSize, fp);
    if(ferror(fp))
    {
        nRet    = FAILURE;
        pszInfo = "load into mem failed";
        goto END;
    }

END:
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    LogInfo(pszInfo);
    return nRet;
}

/************************************************************************/
/* read one line from gs_pszFile
   char **pszLine used to save the beginning of line
   int *pLen      used to save the length of line     

   return SUCCESS if okay
   return FAILED if no more to read                  */
/************************************************************************/
int
ReadLine(char **ppszLine, int *pLen)
{
    //inner state record for gs_pszFile
    static int nCurPos = 0;
    char *pBegin       = gs_pszFile + nCurPos;
    char *pEnd         = gs_pszFile + gs_lFileSize;

    *ppszLine = NULL;
    *pLen     = 0;

    if (pBegin >= pEnd)
    {
        return FAILURE;
    }

    *ppszLine = pBegin;
    while (pBegin[0] != '\r'
          && pBegin[1] != '\n')
    {
        nCurPos++;
        pBegin++;
    }

    //read \r\n
    nCurPos += 2;
    pBegin  += 2;

    *pLen = pBegin - *ppszLine;
    if (*pLen > MAX_LINE)
    {
        LogInfo("this line is too long");
        //perhaps malloc needed.
    }
    else
    {
        //remove \r\n  out of the line
        strncpy(gs_pszLine, *ppszLine, *pLen);
        gs_pszLine[*pLen - 2] = '\0';
        *ppszLine = gs_pszLine;
    }

    return SUCCESS;
}

#if 0

/************************************************************************/
/* recognize words from pszLine
   and do some statistic  

jz      short loc_401883
cmp     [ebp+Dest], 0                                                                   */
/************************************************************************/
static void 
ParseWords(char *pszLine)
{
    char *pToken = NULL;
    char *seps   = " ,\"";

    pToken = strtok( pszLine, seps);
    while( pToken != NULL )
    {
        gs_tagRet.m_map_word_count[pToken]++;
        pToken = strtok( NULL, seps);
    }
}

static int
cmp(const pair<string, int> &x, const pair<string, int> &y)
{
    return x.second > y.second;
}

/************************************************************************/
/* walk through the map, just for test    
   and we do some sorting                                               */
/************************************************************************/
static void
WalkWords(map<string, int> &theContainer)
{
    LogInfo("Walking all the words (word, count)......");

    char szInfo[MAX_LINE];
    sprintf(szInfo, "Total Words: %d", theContainer.size());
    LogInfo(szInfo);
    
    vector<pair<string, int> > vec_pair;

    map<string, int>::iterator it;
    theContainer.value_comp();
    for (it = theContainer.begin(); it != theContainer.end(); it++)
    {
#if 0
        sprintf(szInfo, "(%s, %d)",it->first.data(), it->second);
        LogInfo(szInfo);
#endif
        //do sth sort
        vec_pair.push_back(make_pair(it->first, it->second));
    }   
    
    sort(vec_pair.begin(), vec_pair.end(), cmp);

    vector<pair<string, int> >::iterator it_vec;
    for (it_vec = vec_pair.begin(); it_vec != vec_pair.end(); it_vec++)
    {
        sprintf(szInfo, "(%s, %d)",it_vec->first.data(), it_vec->second);
        LogInfo(szInfo);
    }
}

/************************************************************************/
/* parse the line, and do some statistic                               */
/************************************************************************/
static void
ParseLine(char *pszLine)
{
    if (NULL == pszLine)
    {
        return;
    }

    gs_tagRet.m_nTotalLine++;

    //\r\n
    if (0 == strlen(pszLine))
    {
        gs_tagRet.m_nBlankLine++;
        return;
    }

    //
    SkipSpace(&pszLine);

    //comment line
    if (';' == pszLine[0])
    {
        gs_tagRet.m_nCommentLine++;
        return;
    }

    //comment may also exists in the end of line
    //but now, we just ignore them
    char *p = strstr(pszLine, ";");
    if (p != NULL)
    {
        p[0] = '\0';

        //can also parse words in comments
        ParseWords(&p[1]);
    }

    //parse words now
    ParseWords(pszLine);
}

/************************************************************************/
/* Dump the statistic Result                                            */
/************************************************************************/
static void
DumpResult(void)
{
    char szInfo[MAX_LINE];

    sprintf(szInfo, "Blank Line     : %d\r\n"
                    "Comment Line   : %d\r\n"
                    "Total Line     : %d\r\n", 
                     gs_tagRet.m_nBlankLine,
                     gs_tagRet.m_nCommentLine,
                     gs_tagRet.m_nTotalLine);

    LogInfo(szInfo);

    WalkWords(gs_tagRet.m_map_word_count);
}

/************************************************************************/
/* the key point, read line from the mem and parse line                */
/************************************************************************/
static void
ParseFile()
{
    char *pszLine = NULL;
    int  nLen     = 0;

    while (SUCCESS == ReadLine(&pszLine, &nLen))
    {  
        ParseLine(pszLine);
    }
}

int
test_main(int argc, char **argv)
{
    if (argc != 2)
    {
        Usage();
        return -1;
    }

    if (LoadFile(argv[1]) != SUCCESS)
    {
        return -1;
    }

    ParseFile();

    DumpResult();

    return 0;
}

#endif