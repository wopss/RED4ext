#include "Logger.hpp"
#include "App.hpp"
#include "stdafx.hpp"

#define Log(func)                                                                                                      \
    if (!aMessage)                                                                                                     \
    {                                                                                                                  \
        spdlog::warn("Plugin with handle {} tried to log a message with a NULL message", fmt::ptr(aHandle));           \
        return;                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    auto app = App::Get();                                                                                             \
    if (!app)                                                                                                          \
    {                                                                                                                  \
        return;                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    auto pluginSystem = app->GetPluginSystem();                                                                        \
    auto plugin = pluginSystem->GetPlugin(aHandle);                                                                    \
    if (!plugin)                                                                                                       \
    {                                                                                                                  \
        return;                                                                                                        \
    }                                                                                                                  \
    auto loggerSystem = app->GetLoggerSystem();                                                                        \
    loggerSystem->func(plugin, aMessage)

#define LogF(char_type, count_fn, format_fn, func)                                                                     \
    if (!aFormat)                                                                                                      \
    {                                                                                                                  \
        spdlog::warn("Plugin with handle {} tried to log a message with a NULL format", fmt::ptr(aHandle));            \
        return;                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    auto app = App::Get();                                                                                             \
    if (!app)                                                                                                          \
    {                                                                                                                  \
        return;                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    auto pluginSystem = app->GetPluginSystem();                                                                        \
    auto plugin = pluginSystem->GetPlugin(aHandle);                                                                    \
    if (!plugin)                                                                                                       \
    {                                                                                                                  \
        return;                                                                                                        \
    }                                                                                                                  \
                                                                                                                       \
    va_list args;                                                                                                      \
    va_start(args, aFormat);                                                                                           \
                                                                                                                       \
    auto len = count_fn(aFormat, args);                                                                                \
    if (len > 0)                                                                                                       \
    {                                                                                                                  \
        std::vector<char_type> buffer(len + 1); /* len + NULL character. */                                            \
                                                                                                                       \
        auto res = format_fn(buffer.data(), buffer.size(), buffer.size() - 1, aFormat, args);                          \
        if (res > 0)                                                                                                   \
        {                                                                                                              \
            auto loggerSystem = app->GetLoggerSystem();                                                                \
            loggerSystem->func(plugin, {buffer.data(), buffer.size() - 1}); /* Exclude NULL character. */              \
        }                                                                                                              \
        else if (res < 0)                                                                                              \
        {                                                                                                              \
            spdlog::warn(L"Could not format the log message logged by '{}'. '" #format_fn "' returned {}",             \
                         plugin->GetName(), res);                                                                      \
        }                                                                                                              \
    }                                                                                                                  \
    else if (len < 0)                                                                                                  \
    {                                                                                                                  \
        spdlog::warn(L"Could not get the length of the formatted log message logged by '{}'. '" #count_fn              \
                     "' returned {}",                                                                                  \
                     plugin->GetName(), len);                                                                          \
    }                                                                                                                  \
                                                                                                                       \
    va_end(args)

void v1::Logger::Trace(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Trace);
}

void v1::Logger::TraceF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Trace);
}

void v1::Logger::TraceW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Trace);
}

void v1::Logger::TraceWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Trace);
}

void v1::Logger::Debug(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Debug);
}

void v1::Logger::DebugF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Debug);
}

void v1::Logger::DebugW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Debug);
}

void v1::Logger::DebugWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Debug);
}

void v1::Logger::Info(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Info);
}

void v1::Logger::InfoF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Info);
}

void v1::Logger::InfoW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Info);
}

void v1::Logger::InfoWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Info);
}

void v1::Logger::Warn(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Warn);
}

void v1::Logger::WarnF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Warn);
}

void v1::Logger::WarnW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Warn);
}

void v1::Logger::WarnWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Warn);
}

void v1::Logger::Error(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Error);
}

void v1::Logger::ErrorF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Error);
}

void v1::Logger::ErrorW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Error);
}

void v1::Logger::ErrorWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Error);
}

void v1::Logger::Critical(RED4ext::PluginHandle aHandle, const char* aMessage)
{
    Log(Critical);
}

void v1::Logger::CriticalF(RED4ext::PluginHandle aHandle, const char* aFormat, ...)
{
    LogF(char, ::_vscprintf, ::vsnprintf_s, Critical);
}

void v1::Logger::CriticalW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage)
{
    Log(Critical);
}

void v1::Logger::CriticalWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...)
{
    LogF(wchar_t, ::_vscwprintf, ::_vsnwprintf_s, Critical);
}
