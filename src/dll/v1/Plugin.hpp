#pragma once

#include "PluginBase.hpp"

namespace v1
{
class Plugin final : public PluginBase
{
public:
    Plugin(const std::filesystem::path& aPath, wil::unique_hmodule aModule);

    const uint32_t GetApiVersion() const override;
    void* GetPluginInfo() override;
    const void* GetSdkStruct() const override;

    const std::wstring_view GetName() const override;
    const std::wstring_view GetAuthor() const override;
    const RED4ext::SemVer& GetVersion() const override;
    const RED4ext::FileVer& GetRuntimeVersion() const override;
    const RED4ext::SemVer& GetSdkVersion() const override;

private:
    RED4ext::v1::PluginInfo m_info;

    RED4ext::v1::SemVer m_runtime;
    RED4ext::v1::Logger m_logger;
    RED4ext::v1::Hooking m_hooking;
    RED4ext::v1::GameStates m_gameStates;
    RED4ext::v1::Scripts m_scripts;
    RED4ext::v1::Sdk m_sdk;
};
} // namespace v1
