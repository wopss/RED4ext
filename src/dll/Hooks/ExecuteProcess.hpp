#pragma once

#include <scc.h>

namespace Hooks::ExecuteProcess
{
bool Attach();
bool Detach();
} // namespace Hooks::ExecuteProcess

bool ExecuteScc(SccApi& scc);
