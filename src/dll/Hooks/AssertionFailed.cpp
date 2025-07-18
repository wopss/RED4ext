#include "AssertionFailed.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "stdafx.hpp"

namespace
{

bool isAttached = false;

void _AssertionFailed(const char*, int, const char*, const char*, ...);

Hook<decltype(&_AssertionFailed)> AssertionFailed_fnc(Hashes::AssertionFailed, &_AssertionFailed);

void _AssertionFailed(const char* aFile, int aLineNum, const char* aCondition, const char* aMessage, ...)
{
    spdlog::error("Crash report");
    spdlog::error("------------");
    spdlog::error("File: {}", aFile);
    spdlog::error("Line: {}", aLineNum);

    // Size limit defined by the game.
    char msg[0x400] = "<not supplied>";

    if (aCondition)
    {
        spdlog::error("Condition: {}", aCondition);
    }
    if (aMessage)
    {
        va_list args;
        va_start(args, aMessage);

        vsnprintf_s(msg, sizeof(msg), aMessage, args);
        spdlog::error("Message: {}", msg);

        va_end(args);
    }

    spdlog::error("------------");
    spdlog::details::registry::instance().flush_all();

    AssertionFailed_fnc(aFile, aLineNum, aCondition, msg);
}
} // namespace

bool Hooks::AssertionFailed::Attach()
{
    spdlog::trace("Trying to attach the hook for the assertion failed function at {:#x}...",
                  AssertionFailed_fnc.GetAddress());

    auto result = AssertionFailed_fnc.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the assertion failed function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the assertion failed function was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::AssertionFailed::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the assertion failed function at {:#x}...",
                  AssertionFailed_fnc.GetAddress());

    auto result = AssertionFailed_fnc.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the assertion failed function. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the assertion failed function was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
