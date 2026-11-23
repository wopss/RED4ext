#include "Script_GetBlobName.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

#include <spdlog/spdlog.h>

#include <Windows.h>

#include <cstdint>
#include <filesystem>

namespace
{
bool isAttached = false;

const char* _GetBlobName(std::uint8_t a1);
Hook<decltype(&_GetBlobName)> GetBlobName(Hashes::Script_GetBlobName, &_GetBlobName);

const char* _GetBlobName(std::uint8_t a1)
{
    auto scriptCompilationSystem = App::Get()->GetScriptCompilationSystem();
    if (scriptCompilationSystem->HasModdedScriptsBlob())
    {
        spdlog::info("Redirecting the script blob file name to its '.modded' variant");

        switch (a1)
        {
        case 0:
        {
            return "master.redscripts.modded";
        }
        case 2:
        {
            return "profiling.redscripts.modded";
        }
        case 8:
        {
            return "final.redscripts.modded";
        }
        case 16:
        {
            return "final_noopts.redscripts.modded";
        }
        default:
        {
            return "final.redscripts.modded";
        }
        }
    }

    return GetBlobName(a1);
}
} // namespace

bool Hooks::Script_GetBlobName::Attach()
{
    spdlog::trace("Trying to attach the hook for the blob name at {:#x}...", GetBlobName.GetAddress());

    auto result = GetBlobName.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the blob name. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the blob name was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::Script_GetBlobName::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the blob name at {:#x}...", GetBlobName.GetAddress());

    auto result = GetBlobName.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the blob name. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the blob name was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
