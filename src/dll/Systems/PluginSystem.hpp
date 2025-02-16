#pragma once

#include "Config.hpp"
#include "ISystem.hpp"
#include "Paths.hpp"
#include "PluginBase.hpp"

class PluginSystem final : public ISystem
{
public:
    using PluginName = std::wstring;

    PluginSystem(const Config::PluginsConfig& aConfig, const Paths& aPaths);
    ~PluginSystem() override = default;

    ESystemType GetType() override;

    void Startup() override;
    void Shutdown() override;

    std::shared_ptr<PluginBase> GetPlugin(HMODULE aModule) const;
    const std::vector<PluginName>& GetIncompatiblePlugins() const;
    std::vector<PluginName> GetActivePlugins() const;

private:
    using Map_t = std::unordered_map<HMODULE, std::shared_ptr<PluginBase>>;
    using MapIter_t = Map_t::iterator;

    struct PluginLoadInfo
    {
        std::filesystem::path path;
        bool useAlteredSearchPath;
    };

    void Load(const std::filesystem::path& aPath, bool aUseAlteredSearchPath);
    MapIter_t Unload(std::shared_ptr<PluginBase> aPlugin);

    void EnteredRunningState() const;

    std::shared_ptr<PluginBase> CreatePlugin(const std::filesystem::path& aPath, wil::unique_hmodule aModule) const;

    const Config::PluginsConfig& m_config;
    const Paths& m_paths;

    Map_t m_plugins;
    std::vector<PluginName> m_incompatiblePlugins;
};
