#pragma once

#include "ScriptValidationError.hpp"
#include "Systems/PluginSystem.hpp"

namespace Hooks::ValidateScripts
{
bool Attach();
bool Detach();
} // namespace Hooks::ValidateScripts

std::optional<std::wstring> WritePopupMessage(const std::vector<ValidationError>& validationErrors,
                                              const std::vector<PluginSystem::PluginName>& incompatiblePlugins);
