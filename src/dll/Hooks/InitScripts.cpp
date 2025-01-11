#include "InitScripts.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "Systems/ScriptCompilationSystem.hpp"

namespace
{
bool isAttached = false;

void* _CBaseEngine_InitScripts(RED4ext::CBaseEngine* aThis, const RED4ext::CString& aScriptsBlobPath, int8_t a3,
                               int16_t a4);
Hook<decltype(&_CBaseEngine_InitScripts)> CBaseEngine_InitScripts(Hashes::CBaseEngine_InitScripts,
                                                                  &_CBaseEngine_InitScripts);

void* _CBaseEngine_InitScripts(RED4ext::CBaseEngine* aThis, const RED4ext::CString& aScriptsBlobPath, int8_t a3,
                               int16_t a4)
{
    if (!aScriptsBlobPath.Length())
    {
        return CBaseEngine_InitScripts(aThis, aScriptsBlobPath, a3, a4);
    }

    spdlog::info("Scripts BLOB is set to '{}'", aScriptsBlobPath.c_str());

    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    scriptCompilationSystem->SetScriptsBlob(aScriptsBlobPath.c_str());

    auto result = CBaseEngine_InitScripts(aThis, "", a3, a4);
    aThis->scriptsBlobPath = aScriptsBlobPath;
    return result;
}
} // namespace

bool Hooks::InitScripts::Attach()
{
    spdlog::trace("Trying to attach the hook for init scripts at {:#x}...", CBaseEngine_InitScripts.GetAddress());

    auto result = CBaseEngine_InitScripts.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for init scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for init scripts was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::InitScripts::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for init scripts at {:#x}...", CBaseEngine_InitScripts.GetAddress());

    auto result = CBaseEngine_InitScripts.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for init scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for init scripts was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
