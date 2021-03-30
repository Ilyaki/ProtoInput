#pragma once

// Need these praga comments so the functions are re-exported in the dll project
#pragma comment(linker, "/export:_LockInput")

#pragma comment(linker, "/export:_SuspendExplorer")
#pragma comment(linker, "/export:_RestartExplorer")

#include "protoinpututil.h"