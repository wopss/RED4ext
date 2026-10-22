#include "LoadScriptBlob.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "Systems/ScriptCompilationSystem.hpp"
#include "Utils.hpp"

namespace
{
bool isAttached = false;

// ScriptBundle::LoadFromFile loads a script blob from disk. Since patch 2.5, the game also calls it when reloading
// mod.io script mods in-game, to reload the base blob from the engine's scripts blob path.
//
// That path deliberately still points at the unmodified blob, because the game matches mod blobs to it by file name.
// The catch is that the reload would then load the vanilla scripts and throw away everything scc compiled.
//
// So when the unmodified blob is requested, this hook loads the modded one instead. Everything else, including the
// mod blobs themselves, loads as usual, and the file name matching still sees the original path.

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

    if(!error) {
        return false;
    }

    return aPath.lexically_normal() == unmodified.lexically_normal();
}

bool _ScriptBundle_LoadFromFile(void* aBundle, const RED4ext::CString& aPath)
{
    spdlog::trace("Loading script blob '{}'", aPath.c_str());

    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    if (scriptCompilationSystem->HasModdedScriptsBlob() && IsUnmodifiedScriptsBlob(aPath.c_str()))
    {
        const auto& moddedBlob = scriptCompilationSystem->GetModdedScriptsBlob();
        spdlog::info("Redirecting script blob load from '{}' to '{}'", aPath.c_str(), moddedBlob.string());

        const RED4ext::CString moddedPath(moddedBlob.string().c_str());
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
