#include "Main_Hooks.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "stdafx.hpp"

namespace
{
bool isAttached = false;

int WINAPI _Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
Hook<decltype(&_Main)> Main_fnc(Hashes::Main, &_Main);

int WINAPI _Main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
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

    auto result = Main_fnc(hInstance, hPrevInstance, pCmdLine, nCmdShow);

    try
    {
        auto app = App::Get();
        app->Shutdown();
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

    return result;
}
} // namespace

bool Hooks::Main::Attach()
{
    spdlog::trace("Trying to attach the hook for the main function at {:#x}...", Main_fnc.GetAddress());

    auto result = Main_fnc.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the main function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the main function was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::Main::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the main function at {:#x}...", Main_fnc.GetAddress());

    auto result = Main_fnc.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the main function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the main function was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
