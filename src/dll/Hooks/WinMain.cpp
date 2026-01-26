#include "WinMain.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

namespace
{
bool isAttached = false;

int WINAPI WinMain_fnc(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
Hook<decltype(&WinMain_fnc)> _WinMain(Hashes::WinMain, &WinMain_fnc);

int WINAPI WinMain_fnc(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
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

    return _WinMain(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}
} // namespace

bool Hooks::WinMain::Attach()
{
    spdlog::trace("Trying to attach the hook for the WinMain function at {:#x}...", _WinMain.GetAddress());

    auto result = _WinMain.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the WinMain function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the WinMain function was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::WinMain::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the WinMain function at {:#x}...", _WinMain.GetAddress());

    auto result = _WinMain.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the WinMain function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the WinMain function was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
