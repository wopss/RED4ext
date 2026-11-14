#include "wWinMainCRTStartup.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

namespace
{
bool isAttached = false;

int WINAPI wWinMainCRTStartup_fnc();
Hook<decltype(&wWinMainCRTStartup_fnc)> _wWinMainCRTStartup(Hashes::wWinMainCRTStartup, &wWinMainCRTStartup_fnc);

int WINAPI wWinMainCRTStartup_fnc()
{
    try
    {
        auto app = App::Get();
        app->Startup();
    }
    catch (const std::exception& e)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE("An exception occurred while RED4ext was starting up.\n\n{}",
                                            Utils::Widen(e.what()));
    }
    catch (...)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE("An unknown exception occurred while RED4ext was starting up.");
    }

    return _wWinMainCRTStartup();
}
} // namespace

bool Hooks::wWinMainCRTStartup::Attach()
{
    spdlog::trace("Trying to attach the hook for the wWinMainCRTStartup function at {:#x}...", _wWinMainCRTStartup.GetAddress());

    auto result = _wWinMainCRTStartup.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the wWinMainCRTStartup function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the wWinMainCRTStartup function was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::wWinMainCRTStartup::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the wWinMainCRTStartup function at {:#x}...", _wWinMainCRTStartup.GetAddress());

    auto result = _wWinMainCRTStartup.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the wWinMainCRTStartup function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the wWinMainCRTStartup function was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
