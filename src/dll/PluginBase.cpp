#include "PluginBase.hpp"
#include "Utils.hpp"

#include <RED4ext/Api/v1/EMainReason.hpp>
#include <RED4ext/Api/v1/PluginHandle.hpp>
#include <spdlog/spdlog.h>
#include <wil/resource.h>

#include <Windows.h>

#include <exception>
#include <filesystem>
#include <utility>

PluginBase::PluginBase(const std::filesystem::path& aPath, wil::unique_hmodule aModule)
    : m_path(aPath)
    , m_module(std::move(aModule))
{
}

const std::filesystem::path& PluginBase::GetPath() const
{
    return m_path;
}

HMODULE PluginBase::GetModule() const
{
    return m_module.get();
}

bool PluginBase::Query()
{
    const auto& path = GetPath();
    const auto stem = path.stem();
    const auto module = GetModule();

    spdlog::trace(L"Calling 'Query' function exported by '{}'...", stem);

    using Query_t = void (*)(void*);
    auto queryFn = reinterpret_cast<Query_t>(GetProcAddress(module, "Query"));
    if (!queryFn)
    {
        auto msg = Utils::FormatLastError();
        spdlog::warn(L"Could not retrieve 'Query' function from '{}'. Error code: {}, msg: '{}', path: '{}'", stem,
                     GetLastError(), msg, path);
        return false;
    }

    try
    {
        queryFn(GetPluginInfo());
    }
    catch (const std::exception& e)
    {
        spdlog::warn(L"An exception occured while calling 'Query' function exported by '{}'. Path: '{}'", stem, path);
        spdlog::warn(e.what());
        return false;
    }
    catch (...)
    {
        spdlog::warn(L"An unknown exception occured while calling 'Query' function exported by '{}'. Path: '{}'", stem,
                     path);
        return false;
    }

    auto name = GetName();
    if (name.empty())
    {
        spdlog::warn(L"'{}' does not have a name; one is required. Path: '{}'", stem, path);
        return false;
    }

    auto author = GetAuthor();
    if (author.empty())
    {
        spdlog::warn(L"'{}' does not have any author(s); an author is required. Path: '{}'", stem, path);
        return false;
    }

    spdlog::trace(L"'Query' function called successfully");
    return true;
}

bool PluginBase::Main(RED4ext::v1::EMainReason aReason)
{
    const auto module = GetModule();
    const auto name = GetName();
    const auto reasonStr = aReason == RED4ext::v1::EMainReason::Load ? L"Load" : L"Unload";

    spdlog::trace(L"Calling 'Main' function exported by '{}' with reason '{}'...", name, reasonStr);

    using Main_t = bool (*)(RED4ext::v1::PluginHandle, RED4ext::v1::EMainReason, const void*);
    auto mainFn = reinterpret_cast<Main_t>(GetProcAddress(module, "Main"));
    if (mainFn)
    {
        try
        {
            auto success = mainFn(module, aReason, GetSdkStruct());
            if (!success)
            {
                spdlog::trace(L"'Main' function returned 'false'");
                return false;
            }

            spdlog::trace(L"'Main' function called successfully");
        }
        catch (const std::exception& e)
        {
            spdlog::warn(L"An exception occured while calling 'Main' function with reason '{}', exported by '{}'",
                         reasonStr, name);
            spdlog::warn(e.what());
            return false;
        }
        catch (...)
        {
            spdlog::warn(
                L"An unknown exception occured while calling 'Main' function with reason '{}', exported by '{}'",
                reasonStr, name);
            return false;
        }
    }
    else
    {
        spdlog::trace(L"'{}' does not export a 'Main' function, skipping the call", name);
    }

    return true;
}
