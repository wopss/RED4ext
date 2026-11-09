#include "LoadScripts.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "Systems/ScriptCompilationSystem.hpp"

#include <fmt/format.h>
#include <fmt/std.h>

#include <string>

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
    const auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();

    if (scriptCompilationSystem->HasModdedScriptsBlob())
    {
        const RED4ext::CString blobBath = fmt::format("{}", scriptCompilationSystem->GetModdedScriptsBlob());
        return CBaseEngine_LoadScripts(aEngine, blobBath, aTimestamp, a4);
    }
    else if (scriptCompilationSystem->HasScriptsBlob())
    {
        const RED4ext::CString blobBath = fmt::format("{}", scriptCompilationSystem->GetScriptsBlob());
        return CBaseEngine_LoadScripts(aEngine, blobBath, aTimestamp, a4);
    }

    return CBaseEngine_LoadScripts(aEngine, aPath, aTimestamp, a4);
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
