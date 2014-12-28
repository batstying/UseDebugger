#include "UseDebugger.h"

int
main(void)
{
    CUseDebugger *pDebugger = CUseDebugger::CreateSystem();
    if (NULL == pDebugger)
    {
        return -1;
    }

    pDebugger->Run();
    pDebugger->DestorySystem();

    return 0;
}