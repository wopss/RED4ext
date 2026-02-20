#include "v1/Plugin.hpp"
#include "Image.hpp"
#include "stdafx.hpp"
#include "v1/Funcs.hpp"
#include "v1/Logger.hpp"

v1::Plugin::Plugin(const std::filesystem::path& aPath, wil::unique_hmodule aModule)
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

    m_logger.Trace = v1::Logger::Trace;
    m_logger.TraceF = v1::Logger::TraceF;
    m_logger.TraceW = v1::Logger::TraceW;
    m_logger.TraceWF = v1::Logger::TraceWF;
    m_logger.Debug = v1::Logger::Debug;
    m_logger.DebugF = v1::Logger::DebugF;
    m_logger.DebugW = v1::Logger::DebugW;
    m_logger.DebugWF = v1::Logger::DebugWF;
    m_logger.Info = v1::Logger::Info;
    m_logger.InfoF = v1::Logger::InfoF;
    m_logger.InfoW = v1::Logger::InfoW;
    m_logger.InfoWF = v1::Logger::InfoWF;
    m_logger.Warn = v1::Logger::Warn;
    m_logger.WarnF = v1::Logger::WarnF;
    m_logger.WarnWF = v1::Logger::WarnWF;
    m_logger.WarnW = v1::Logger::WarnW;
    m_logger.Error = v1::Logger::Error;
    m_logger.ErrorF = v1::Logger::ErrorF;
    m_logger.ErrorW = v1::Logger::ErrorW;
    m_logger.ErrorWF = v1::Logger::ErrorWF;
    m_logger.Critical = v1::Logger::Critical;
    m_logger.CriticalF = v1::Logger::CriticalF;
    m_logger.CriticalW = v1::Logger::CriticalW;
    m_logger.CriticalWF = v1::Logger::CriticalWF;

    m_hooking.Attach = v1::Hooking::Attach;
    m_hooking.Detach = v1::Hooking::Detach;

    m_gameStates.Add = v1::GameStates::Add;

    m_scripts.Add = v1::Scripts::Add;
    m_scripts.RegisterNeverRefType = v1::Scripts::RegisterNeverRefType;
    m_scripts.RegisterMixedRefType = v1::Scripts::RegisterMixedRefType;
}

const uint32_t v1::Plugin::GetApiVersion() const
{
    return RED4EXT_API_VERSION_1;
}

void* v1::Plugin::GetPluginInfo()
{
    return &m_info;
}

const void* v1::Plugin::GetSdkStruct() const
{
    return &m_sdk;
}

const std::wstring_view v1::Plugin::GetName() const
{
    return m_info.name;
}

const std::wstring_view v1::Plugin::GetAuthor() const
{
    return m_info.author;
}

const RED4ext::SemVer& v1::Plugin::GetVersion() const
{
    return m_info.version;
}

const RED4ext::FileVer& v1::Plugin::GetRuntimeVersion() const
{
    return m_info.runtime;
}

const RED4ext::SemVer& v1::Plugin::GetSdkVersion() const
{
    return m_info.sdk;
}
