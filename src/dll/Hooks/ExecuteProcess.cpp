#include "ExecuteProcess.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "ScriptCompiler/ScriptCompilerSettings.hpp"
#include "Systems/ScriptCompilationSystem.hpp"
#include <windows.h>

namespace
{
bool isAttached = false;

bool _Global_ExecuteProcess(void* a1, RED4ext::CString& aCommand, FixedWString& aArgs,
                            RED4ext::CString& aCurrentDirectory, char a5);
Hook<decltype(&_Global_ExecuteProcess)> Global_ExecuteProcess(Hashes::Global_ExecuteProcess, &_Global_ExecuteProcess);

bool _Global_ExecuteProcess(void* a1, RED4ext::CString& aCommand, FixedWString& aArgs,
                            RED4ext::CString& aCurrentDirectory, char a5)
{
    if (strstr(aCommand.c_str(), "scc.exe") == nullptr)
    {
        return Global_ExecuteProcess(a1, aCommand, aArgs, aCurrentDirectory, a5);
    }

    auto sccPath = std::filesystem::path(aCommand.c_str());
    auto& sccLib = sccPath.replace_filename("scc_lib.dll");
    auto sccHandle = LoadLibrary(sccLib.c_str());
    if (sccHandle)
    {
        auto scc = scc_load_api(sccHandle);
        return ExecuteScc(scc);
    }

    spdlog::info(L"Could not load the scc library from '{}', falling back to the CLI", sccLib.native());

    auto str = App::Get()->GetScriptCompilationSystem()->GetCompilationArgs(aArgs);

    FixedWString newArgs{};
    newArgs.str = str.c_str();
    newArgs.length = static_cast<std::uint32_t>(str.length());
    newArgs.maxLength = static_cast<std::uint32_t>(str.capacity());
    spdlog::info(L"Final redscript compilation arg string: '{}'", newArgs.str);
    return Global_ExecuteProcess(a1, aCommand, newArgs, aCurrentDirectory, a5);
}
} // namespace

bool Hooks::ExecuteProcess::Attach()
{
    spdlog::trace("Trying to attach the hook for execute process at {:#x}...", Global_ExecuteProcess.GetAddress());

    auto result = Global_ExecuteProcess.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for execute process. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for execute process was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::ExecuteProcess::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for execute process at {:#x}...", Global_ExecuteProcess.GetAddress());

    auto result = Global_ExecuteProcess.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for execute process. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for execute process was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}

bool ExecuteScc(SccApi& scc)
{
    auto scriptSystem = App::Get()->GetScriptCompilationSystem();
    auto engine = RED4ext::CGameEngine::Get();

    ScriptCompilerSettings settings(scc, App::Get()->GetPaths()->GetR6Dir());

    const auto blobPath = scriptSystem->HasScriptsBlob() ? scriptSystem->GetScriptsBlob()
                                                         : App::Get()->GetPaths()->GetDefaultScriptsBlob();

    auto moddedCacheFile = blobPath;
    moddedCacheFile.replace_extension("redscripts.modded");

    if (scriptSystem->HasScriptsBlob())
    {
        settings.SetCustomCacheFile(blobPath);
    }

    if (settings.SupportsOutputCacheFileParameter())
    {
        settings.SetOutputCacheFile(moddedCacheFile);
    }

    for (const auto& [_, path] : scriptSystem->GetScriptPaths())
    {
        settings.AddScriptPath(path);
    }

    for (const auto& type : scriptSystem->GetNeverRefTypes())
    {
        settings.RegisterNeverRefType(type);
    }

    for (const auto& type : scriptSystem->GetMixedRefTypes())
    {
        settings.RegisterMixedRefType(type);
    }

    const auto result = settings.Compile();

    if (const auto error = std::get_if<ScriptCompilerFailure>(&result))
    {
        engine->scriptsCompilationErrors = error->GetMessage().c_str();
        spdlog::warn("scc invocation failed with an error: {}", error->GetMessage());
        return false;
    }

    auto& sourceRepo = scriptSystem->GetSourceRefRepository();

    const auto& output = std::get<ScriptCompilerOutput>(result);
    const size_t refCount = output.GetSourceRefCount();

    for (size_t i = 0; i < refCount; ++i)
    {
        const auto sccRef = output.GetSourceRef(i);
        if (!sccRef.IsNative())
        {
            continue;
        }

        const SourceRef sourceRef{.file = sourceRepo.RegisterSourceFile(sccRef.GetPath()), .line = sccRef.GetLine()};

        switch (sccRef.GetType())
        {
        case SCC_SOURCE_REF_TYPE_CLASS:
            sourceRepo.RegisterClass(sccRef.GetName(), sourceRef);
            break;
        case SCC_SOURCE_REF_TYPE_FIELD:
            sourceRepo.RegisterProperty(sccRef.GetName(), sccRef.GetParentName(), sourceRef);
            break;
        case SCC_SOURCE_REF_TYPE_FUNCTION:
            auto parentName = sccRef.GetParentName();
            if (parentName.empty())
            {
                sourceRepo.RegisterFunction(sccRef.GetName(), sourceRef);
            }
            else
            {
                sourceRepo.RegisterMethod(sccRef.GetName(), parentName, sourceRef);
            }
            break;
        }
    }

    spdlog::info("scc invoked successfully, {} source refs were registered", refCount);

    if (settings.SupportsOutputCacheFileParameter())
    {
        scriptSystem->SetModdedScriptsBlob(moddedCacheFile);
        engine->scriptsBlobPath = Utils::Narrow(moddedCacheFile.c_str());
        spdlog::info(L"Scripts blob path was updated to '{}'", moddedCacheFile);
    }

    return true;
}
