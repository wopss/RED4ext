#include "Plugin.hpp"

#include "Funcs.hpp"
#include "Image.hpp"
#include "Logger.hpp"

namespace v1
{
Plugin::Plugin(const std::filesystem::path& aPath, wil::unique_hmodule aModule)
    : PluginBase(aPath, std::move(aModule))
    , m_info{}
    , m_runtime(Image::Get()->GetProductVersion())
    , m_logger{.Trace = Logger::Trace,
               .TraceF = Logger::TraceF,
               .TraceW = Logger::TraceW,
               .TraceWF = Logger::TraceWF,
               .Debug = Logger::Debug,
               .DebugF = Logger::DebugF,
               .DebugW = Logger::DebugW,
               .DebugWF = Logger::DebugWF,
               .Info = Logger::Info,
               .InfoF = Logger::InfoF,
               .InfoW = Logger::InfoW,
               .InfoWF = Logger::InfoWF,
               .Warn = Logger::Warn,
               .WarnF = Logger::WarnF,
               .WarnW = Logger::WarnW,
               .WarnWF = Logger::WarnWF,
               .Error = Logger::Error,
               .ErrorF = Logger::ErrorF,
               .ErrorW = Logger::ErrorW,
               .ErrorWF = Logger::ErrorWF,
               .Critical = Logger::Critical,
               .CriticalF = Logger::CriticalF,
               .CriticalW = Logger::CriticalW,
               .CriticalWF = Logger::CriticalWF}
    , m_hooking{.Attach = Hooking::Attach, .Detach = Hooking::Detach}
    , m_gameStates{.AddHook = GameStates::AddHook}
    , m_scripts{.Add = Scripts::Add}
    , m_sdk{.runtime = &m_runtime,
            .logger = &m_logger,
            .hooking = &m_hooking,
            .gameStates = &m_gameStates,
            .scripts = &m_scripts}
{
}

const uint32_t Plugin::GetApiVersion() const
{
    return RED4EXT_API_VERSION_1;
}

void* Plugin::GetPluginInfo()
{
    return &m_info;
}

const void* Plugin::GetSdkStruct() const
{
    return &m_sdk;
}

const std::wstring_view Plugin::GetName() const
{
    return m_info.name;
}

const std::wstring_view Plugin::GetAuthor() const
{
    return m_info.author;
}

const RED4ext::SemVer& Plugin::GetVersion() const
{
    return m_info.version;
}

const RED4ext::FileVer& Plugin::GetRuntimeVersion() const
{
    return m_info.runtime;
}

const RED4ext::SemVer& Plugin::GetSdkVersion() const
{
    return m_info.sdk;
}
} // namespace v1
