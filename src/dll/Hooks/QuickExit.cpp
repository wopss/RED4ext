#include "QuickExit.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

#include <cstdint>

namespace
{
bool isAttached = false;

int WINAPI _QuickExit_fnc(std::int32_t aExitCode);
Hook<decltype(&_QuickExit_fnc)> _QuickExit(Hashes::QuickExit, &_QuickExit_fnc);

int WINAPI _QuickExit_fnc(std::int32_t aExitCode)
{
    try
    {
        auto app = App::Get();
        if (app)
        {
            app->Shutdown();
        }
    }
    catch (const std::exception& e)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE("An exception occurred while RED4ext was shutting down.\n\n{}",
                                            Utils::Widen(e.what()));
    }
    catch (...)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE("An unknown exception occurred while RED4ext was shutting down.");
    }

    return _QuickExit(aExitCode);
}
} // namespace

bool Hooks::QuickExit::Attach()
{
    spdlog::trace("Trying to attach the hook for the 'quick_exit' function at {:#x}...", _QuickExit.GetAddress());

    auto result = _QuickExit.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the 'quick_exit' function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the 'quick_exit' function was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::QuickExit::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the 'quick_exit' function at {:#x}...", _QuickExit.GetAddress());

    auto result = _QuickExit.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the 'quick_exit' function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the 'quick_exit' function was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
