#pragma once

// Need these praga comments so the functions are re-exported in the dll project
#ifdef _WIN64
#pragma comment(linker, "/export:LockInput")

#pragma comment(linker, "/export:SuspendExplorer")
#pragma comment(linker, "/export:RestartExplorer")
#else
#pragma comment(linker, "/export:_LockInput")

#pragma comment(linker, "/export:_SuspendExplorer")
#pragma comment(linker, "/export:_RestartExplorer")
#endif



#include "protoinpututil.h"