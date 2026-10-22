#include "LoadScriptBlob.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "Systems/ScriptCompilationSystem.hpp"
#include "Utils.hpp"

namespace
{
bool isAttached = false;

// Loads a script blob from a file into a script bundle. Since patch 2.5 the in-game mod reload (mod.io script mods)
// uses it to load the base blob from the engine's scripts blob path, which stays the unmodified blob so that mod blobs
// match it by file name. Loads of the unmodified blob are redirected to the modded one, otherwise the reload would
// discard everything scc compiled; mod blobs and the scanner's name matching are unaffected.
bool _ScriptBundle_LoadFromFile(void* aBundle, const RED4ext::CString& aPath);
Hook<decltype(&_ScriptBundle_LoadFromFile)> ScriptBundle_LoadFromFile(Hashes::ScriptBundle_LoadFromFile,
                                                                      &_ScriptBundle_LoadFromFile);

bool IsUnmodifiedScriptsBlob(const std::filesystem::path& aPath)
{
    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    const auto unmodified = scriptCompilationSystem->HasScriptsBlob() ? scriptCompilationSystem->GetScriptsBlob()
                                                                      : App::Get()->GetPaths()->GetDefaultScriptsBlob();

    std::error_code error;
    if (std::filesystem::equivalent(aPath, unmodified, error))
    {
        return true;
    }

    return !error ? false : aPath.lexically_normal() == unmodified.lexically_normal();
}

bool _ScriptBundle_LoadFromFile(void* aBundle, const RED4ext::CString& aPath)
{
    spdlog::trace(L"Loading script blob '{}'", Utils::Widen(aPath.c_str()));

    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    if (scriptCompilationSystem->HasModdedScriptsBlob() && IsUnmodifiedScriptsBlob(aPath.c_str()))
    {
        const auto& moddedBlob = scriptCompilationSystem->GetModdedScriptsBlob();
        spdlog::info(L"Redirecting script blob load from '{}' to '{}'", Utils::Widen(aPath.c_str()), moddedBlob);

        const RED4ext::CString moddedPath(Utils::Narrow(moddedBlob.c_str()).c_str());
        return ScriptBundle_LoadFromFile(aBundle, moddedPath);
    }

    return ScriptBundle_LoadFromFile(aBundle, aPath);
}
} // namespace

bool Hooks::LoadScriptBlob::Attach()
{
    spdlog::trace("Trying to attach the hook for load script blob at {:#x}...", ScriptBundle_LoadFromFile.GetAddress());

    auto result = ScriptBundle_LoadFromFile.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for load script blob. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for load script blob was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::LoadScriptBlob::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for load script blob at {:#x}...", ScriptBundle_LoadFromFile.GetAddress());

    auto result = ScriptBundle_LoadFromFile.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for load script blob. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for load script blob was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
