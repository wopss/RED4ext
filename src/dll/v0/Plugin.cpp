#include "v0/Plugin.hpp"
#include "Image.hpp"
#include "stdafx.hpp"
#include "v0/Funcs.hpp"
#include "v0/Logger.hpp"

v0::Plugin::Plugin(const std::filesystem::path& aPath, wil::unique_hmodule aModule)
    : PluginBase(aPath, std::move(aModule))
    , m_info{}
    , m_sdk{}
    , m_runtime(Image::Get()->GetProductVersion())
    , m_logger{}
    , m_hooking{}
    , m_gameStates{}
    , m_scripts{}
{
    m_sdk.runtime = &m_runtime;
    m_sdk.logger = &m_logger;
    m_sdk.hooking = &m_hooking;
    m_sdk.gameStates = &m_gameStates;
    m_sdk.scripts = &m_scripts;

    m_logger.Trace = v0::Logger::Trace;
    m_logger.TraceF = v0::Logger::TraceF;
    m_logger.TraceW = v0::Logger::TraceW;
    m_logger.TraceWF = v0::Logger::TraceWF;
    m_logger.Debug = v0::Logger::Debug;
    m_logger.DebugF = v0::Logger::DebugF;
    m_logger.DebugW = v0::Logger::DebugW;
    m_logger.DebugWF = v0::Logger::DebugWF;
    m_logger.Info = v0::Logger::Info;
    m_logger.InfoF = v0::Logger::InfoF;
    m_logger.InfoW = v0::Logger::InfoW;
    m_logger.InfoWF = v0::Logger::InfoWF;
    m_logger.Warn = v0::Logger::Warn;
    m_logger.WarnF = v0::Logger::WarnF;
    m_logger.WarnWF = v0::Logger::WarnWF;
    m_logger.WarnW = v0::Logger::WarnW;
    m_logger.Error = v0::Logger::Error;
    m_logger.ErrorF = v0::Logger::ErrorF;
    m_logger.ErrorW = v0::Logger::ErrorW;
    m_logger.ErrorWF = v0::Logger::ErrorWF;
    m_logger.Critical = v0::Logger::Critical;
    m_logger.CriticalF = v0::Logger::CriticalF;
    m_logger.CriticalW = v0::Logger::CriticalW;
    m_logger.CriticalWF = v0::Logger::CriticalWF;

    m_hooking.Attach = v0::Hooking::Attach;
    m_hooking.Detach = v0::Hooking::Detach;

    m_gameStates.Add = v0::GameStates::Add;

    m_scripts.Add = v0::Scripts::Add;
    m_scripts.RegisterNeverRefType = v0::Scripts::RegisterNeverRefType;
    m_scripts.RegisterMixedRefType = v0::Scripts::RegisterMixedRefType;
}

const uint32_t v0::Plugin::GetApiVersion() const
{
    return RED4EXT_API_VERSION_0;
}

void* v0::Plugin::GetPluginInfo()
{
    return &m_info;
}

const void* v0::Plugin::GetSdkStruct() const
{
    return &m_sdk;
}

const std::wstring_view v0::Plugin::GetName() const
{
    return m_info.name;
}

const std::wstring_view v0::Plugin::GetAuthor() const
{
    return m_info.author;
}

const RED4ext::SemVer& v0::Plugin::GetVersion() const
{
    return m_info.version;
}

const RED4ext::FileVer& v0::Plugin::GetRuntimeVersion() const
{
    return m_info.runtime;
}

const RED4ext::SemVer& v0::Plugin::GetSdkVersion() const
{
    return m_info.sdk;
}
