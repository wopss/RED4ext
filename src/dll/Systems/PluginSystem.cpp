#include "PluginSystem.hpp"

#include "App.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include "Version.hpp"
#include "v1/Plugin.hpp"

#define MINIMUM_API_VERSION RED4EXT_API_VERSION_1
#define LATEST_API_VERSION RED4EXT_API_VERSION_LATEST

#define MINIMUM_SDK_VERSION RED4EXT_SDK_0_5_0
#define LATEST_SDK_VERSION RED4EXT_SDK_LATEST

#define LOG_FS_ERROR(text, ec)                                                                                         \
    auto val = ec.value();                                                                                             \
    const auto& category = ec.category();                                                                              \
    auto msg = category.message(val);                                                                                  \
    spdlog::error(text L". Error code: {}, msg: '{}'", val, Utils::Widen(msg))

#define LOG_FS_ENTRY_ERROR(text, entry, ec)                                                                            \
    auto val = ec.value();                                                                                             \
    const auto& category = ec.category();                                                                              \
    auto msg = category.message(val);                                                                                  \
    spdlog::error(text L". Error code: {}, msg: '{}', entry: '{}'", val, Utils::Widen(msg), entry)

PluginSystem::PluginSystem(const Config::PluginsConfig& aConfig, const Paths& aPaths)
    : m_config(aConfig)
    , m_paths(aPaths)
{
}

ESystemType PluginSystem::GetType()
{
    return ESystemType::Plugin;
}

void PluginSystem::Startup()
{
    if (!m_config.isEnabled)
    {
        return;
    }

    spdlog::info("Loading plugins...");

    std::error_code ec;

    auto dir = m_paths.GetPluginsDir();
    auto exists = std::filesystem::exists(dir, ec);
    if (ec)
    {
        LOG_FS_ERROR("Could not check the plugins directory existence", ec);
        return;
    }

    if (!exists)
    {
        std::filesystem::create_directories(dir, ec);
        if (ec)
        {
            LOG_FS_ERROR("Could not create the plugins directory", ec);
            return;
        }
    }

    auto iter = std::filesystem::recursive_directory_iterator(
        dir, std::filesystem::directory_options::follow_directory_symlink, ec);

    if (ec)
    {
        LOG_FS_ERROR("Could not create an iterator for the plugins directory", ec);
        return;
    }

    std::vector<PluginLoadInfo> pluginInfos;

    auto end = std::filesystem::end(iter);
    for (; iter != end; iter.increment(ec))
    {
        /*
         * Do not allow something like:
         *  game_root/read4ext/plugins/some_plugin/a_sub_dir/some_plugin.dll
         *  game_root/read4ext/plugins/some_plugin/a_sub_dir/another_sub_dir/some_plugin.dll
         */
        int32_t depth = iter.depth();
        if (depth > 1)
        {
            iter.disable_recursion_pending();
            continue;
        }

        const auto& entry = *iter;
        const auto& path = entry.path();

        if (ec)
        {
            LOG_FS_ENTRY_ERROR(L"Could not fetch a directory entry", path, ec);
            continue;
        }

        // Deal with broken symlinks.
        if (entry.is_symlink(ec))
        {
            if (!entry.exists(ec))
            {
                // Symlink is broken, skip it.
                spdlog::warn(L"Symbolic link is broken, it will be skipped. Symlink: '{}'", path);
                continue;
            }
            else if (ec)
            {
                LOG_FS_ENTRY_ERROR(L"Could not check if the symlink exists", path, ec);
                continue;
            }
        }
        else if (ec)
        {
            LOG_FS_ENTRY_ERROR(L"Could not check if the entry is a symbolic link", path, ec);
            continue;
        }

        if (entry.is_regular_file(ec) && path.extension() == L".dll")
        {
            const auto stem = path.stem();
            if (m_config.ignored.contains(stem))
            {
                spdlog::debug(L"Skipping loading '{}', the plugin is ignored by the user. Path: '{}", stem, path);
                continue;
            }

            bool useAlteredSearchPath = depth == 1;

            pluginInfos.push_back({path, useAlteredSearchPath});
        }
        else if (ec)
        {
            LOG_FS_ENTRY_ERROR(L"Could not check if the entry is a regular file", path, ec);
        }
    }

    // Load plugins after iterating with filesystem. Allow plugins to change their
    // directory's structure without breaking loading of other plugins.
    for (const auto& pluginInfo : pluginInfos)
    {
        Load(pluginInfo.path, pluginInfo.useAlteredSearchPath);
    }

    // In the case where the exe is hosted, check if the host exe has RED4Ext exports.
    Load(m_paths.GetExe(), false);

    spdlog::info("{} plugin(s) loaded", m_plugins.size());

    // Install CRunningState::OnAfterEnter hook for notifying plugins about entry to Running state.
    RED4ext::GameState stateHook{.OnAfterEnter = [](RED4ext::CGameApplication&)
                                 {
                                     App::Get()->GetPluginSystem()->EnteredRunningState();
                                     return RED4ext::EGameStateResult::Finished;
                                 }};
    App::Get()->GetStateSystem()->AddHook(nullptr, RED4ext::EGameStateType::Running, stateHook);
}

void PluginSystem::Shutdown()
{
    if (!m_config.isEnabled)
    {
        return;
    }

    auto size = m_plugins.size();
    spdlog::trace("Unloading {} plugin(s)...", size);

    for (auto it = m_plugins.begin(); it != m_plugins.end();)
    {
        it = Unload(it->second);
    }

    spdlog::info("{} plugin(s) unloaded", size);
}

void PluginSystem::EnteredRunningState() const
{
    if (!m_config.isEnabled)
    {
        return;
    }

    auto size = m_plugins.size();
    spdlog::trace("Notifying {} plugin(s) about entry to Running state...", size);

    for (const auto& plugin : m_plugins | std::views::values)
    {
        plugin->Main(RED4ext::EMainReason::Run);
    }

    spdlog::info("{} plugin(s) notified about entry to Running state", size);
}

std::shared_ptr<PluginBase> PluginSystem::GetPlugin(HMODULE aModule) const
{
    auto iter = m_plugins.find(aModule);
    if (iter != m_plugins.end())
    {
        return iter->second;
    }

    spdlog::warn("Could not find a plugin with handle {}", fmt::ptr(aModule));
    return nullptr;
}

const std::vector<PluginSystem::PluginName>& PluginSystem::GetIncompatiblePlugins() const
{
    return m_incompatiblePlugins;
}

std::vector<PluginSystem::PluginName> PluginSystem::GetActivePlugins() const
{
    std::vector<PluginSystem::PluginName> plugins;

    plugins.reserve(m_plugins.size());
    for (const auto& plugin : m_plugins | std::views::values)
    {
        plugins.emplace_back(plugin->GetName());
    }
    return plugins;
}

void PluginSystem::Load(const std::filesystem::path& aPath, bool aUseAlteredSearchPath)
{
    spdlog::info(L"Loading plugin from '{}'...", aPath);

    uint32_t flags = 0;
    if (aUseAlteredSearchPath)
    {
        flags = LOAD_WITH_ALTERED_SEARCH_PATH;
    }

    const auto stem = aPath.stem();

    wil::unique_hmodule handle;
    if (aPath.extension() == L".exe")
        handle.reset(GetModuleHandleA(nullptr));
    else
        handle.reset(LoadLibraryEx(aPath.c_str(), nullptr, flags));

    if (!handle)
    {
        auto msg = Utils::FormatLastError();
        spdlog::warn(L"Could not load plugin '{}'. Error code: {}, msg: '{}', path: '{}'", stem, GetLastError(), msg,
                     aPath);
        return;
    }

    spdlog::trace(L"'{}' has been loaded into the address space at {}", stem, fmt::ptr(handle.get()));

    auto plugin = CreatePlugin(aPath, std::move(handle));
    if (!plugin)
    {
        // Unsupported API version.
        return;
    }

    if (!plugin->Query())
    {
        // Query failed or some data is invalid.
        return;
    }

    const auto pluginName = plugin->GetName();
    const auto& pluginVersion = plugin->GetVersion();

    const auto image = Image::Get();

    const auto& requestedRuntime = plugin->GetRuntimeVersion();
    if (requestedRuntime != RED4EXT_RUNTIME_INDEPENDENT)
    {
        // Check if the plugins is compiled for a supported version.
        bool isSupported = false;
        const auto supportedVers = image->GetSupportedVersions();

        for (const auto& supportedRuntime : supportedVers)
        {
            if (supportedRuntime == requestedRuntime)
            {
                isSupported = true;
                break;
            }
        }

        if (!isSupported)
        {
            spdlog::warn(
                L"{} (version: {}) is incompatible with the current patch. The requested runtime of the plugin is {}",
                pluginName, std::to_wstring(pluginVersion), requestedRuntime);

            m_incompatiblePlugins.emplace_back(pluginName);
            return;
        }
    }

    const auto& pluginSdk = plugin->GetSdkVersion();
    if (pluginSdk < MINIMUM_SDK_VERSION || pluginSdk > LATEST_SDK_VERSION)
    {
        spdlog::warn(L"{} (version: {}) uses RED4ext.SDK v{} which is not supported by RED4ext v{}. If you are the "
                     L"plugin's author, recompile the plugin with a version of RED4ext.SDK that meets the following "
                     L"criteria: RED4ext.SDK >= {} && RED4ext.SDK <= {}",
                     pluginName, std::to_wstring(pluginVersion), std::to_wstring(pluginSdk), TEXT(RED4EXT_VERSION_STR),
                     std::to_wstring(MINIMUM_SDK_VERSION), std::to_wstring(LATEST_SDK_VERSION));
        return;
    }

    auto module = plugin->GetModule();
    m_plugins.emplace(module, plugin);

    if (!plugin->Main(RED4ext::EMainReason::Load))
    {
        spdlog::warn(L"{} did not initialize properly, unloading...", pluginName);
        Unload(plugin);

        return;
    }

    spdlog::info(L"{} (version: {}, author(s): {}) has been loaded", pluginName, std::to_wstring(pluginVersion),
                 plugin->GetAuthor());
}

PluginSystem::MapIter_t PluginSystem::Unload(std::shared_ptr<PluginBase> aPlugin)
{
    aPlugin->Main(RED4ext::EMainReason::Unload);

    auto module = aPlugin->GetModule();
    auto iter = m_plugins.find(module);
    auto result = m_plugins.erase(iter);

    spdlog::info(L"{} has been unloaded", aPlugin->GetName());
    return result;
}

std::shared_ptr<PluginBase> PluginSystem::CreatePlugin(const std::filesystem::path& aPath,
                                                       wil::unique_hmodule aModule) const
{
    const auto stem = aPath.stem();

    using Supports_t = uint32_t (*)();
    auto supportsFn = reinterpret_cast<Supports_t>(GetProcAddress(aModule.get(), "Supports"));
    if (!supportsFn)
    {
        // If 'Supports' doesn't exists then the plugin might not be a RED4ext plugin. It might be a dependency.
        auto err = GetLastError();
        if (err != ERROR_PROC_NOT_FOUND)
        {
            auto msg = Utils::FormatLastError();
            spdlog::warn(L"Could not retrieve 'Supports' function from '{}'. Error code: {}, msg: '{}', path: '{}'",
                         stem, GetLastError(), msg, aPath);
        }

        return nullptr;
    }

    uint32_t apiVersion;

    try
    {
        apiVersion = supportsFn();
    }
    catch (const std::exception& e)
    {
        spdlog::warn(L"An exception occurred while calling 'Supports' function exported by '{}'. Path: '{}'", stem,
                     aPath);
        spdlog::warn(e.what());
        return nullptr;
    }
    catch (...)
    {
        spdlog::warn(L"An unknown exception occurred while calling 'Supports' function exported by '{}'. Path: '{}'",
                     stem, aPath);
        return nullptr;
    }

    if (apiVersion < MINIMUM_API_VERSION || apiVersion > LATEST_API_VERSION)
    {
        spdlog::warn(L"'{}' is using an unsupported API version. API version: {}, path: '{}'", stem, apiVersion, aPath);
        return nullptr;
    }

    switch (apiVersion)
    {
    case RED4EXT_API_VERSION_1:
    {
        return std::make_shared<v1::Plugin>(aPath, std::move(aModule));
    }
    }

    return nullptr;
}
