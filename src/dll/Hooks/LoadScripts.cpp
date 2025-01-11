#include "LoadScripts.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "Systems/ScriptCompilationSystem.hpp"

namespace
{
bool isAttached = false;

bool _CBaseEngine_LoadScripts(RED4ext::CBaseEngine* aEngine, const RED4ext::CString& aPath, uint64_t aTimestamp,
                              uint64_t a4);
Hook<decltype(&_CBaseEngine_LoadScripts)> CBaseEngine_LoadScripts(Hashes::CBaseEngine_LoadScripts,
                                                                  &_CBaseEngine_LoadScripts);

bool _CBaseEngine_LoadScripts(RED4ext::CBaseEngine* aEngine, const RED4ext::CString& aPath, uint64_t aTimestamp,
                              uint64_t a4)
{
    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    const auto& scriptsBlobPath =
        scriptCompilationSystem->HasModdedScriptsBlob() ? scriptCompilationSystem->GetModdedScriptsBlob().string()
        : scriptCompilationSystem->HasScriptsBlob()     ? scriptCompilationSystem->GetScriptsBlob().string()
                                                        : aPath;

    return CBaseEngine_LoadScripts(aEngine, scriptsBlobPath, aTimestamp, a4);
}
} // namespace

bool Hooks::LoadScripts::Attach()
{
    spdlog::trace("Trying to attach the hook for load scripts at {:#x}...", CBaseEngine_LoadScripts.GetAddress());

    auto result = CBaseEngine_LoadScripts.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for load scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for load scripts was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::LoadScripts::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for load scripts at {:#x}...", CBaseEngine_LoadScripts.GetAddress());

    auto result = CBaseEngine_LoadScripts.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for load scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for load scripts was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
