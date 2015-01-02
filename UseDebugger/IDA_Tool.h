#ifndef IDA_TOOL_H_B36BA4CF_9FAD_4046_9D22_343092F02BB5
#define IDA_TOOL_H_B36BA4CF_9FAD_4046_9D22_343092F02BB5

#define SUCCESS     0x00
#define NOT_EXIST   0x01
#define BAD_PARAM   0x02
#define FAILURE      0x03    //other reason

#define MAX_LINE    1024

/************************************************************************/
/* Try to load file into memory,
   return SUCCESS if okay
   return BAD_PARAM if pszFileName is NULL
   return NOT_EXIST if open file error
   return FAILED for other reason

*/
/************************************************************************/
int
LoadFile(char *pszFileName);

/************************************************************************/
/* read one line from gs_pszFile
   char **pszLine used to save the beginning of line
   int *pLen      used to save the length of line     

   return SUCCESS if okay
   return FAILED if no more to read                  */
/************************************************************************/
int
ReadLine(char **ppszLine, int *pLen);
#endif