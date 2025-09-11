#include "App.hpp"
#include "Addresses.hpp"
#include "DetourTransaction.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include "Version.hpp"

#include "Hooks/AssertionFailed.hpp"
#include "Hooks/CGameApplication.hpp"
#include "Hooks/CollectSaveableSystems.hpp"
#include "Hooks/ExecuteProcess.hpp"
#include "Hooks/InitScripts.hpp"
#include "Hooks/LoadScripts.hpp"
#include "Hooks/Main_Hooks.hpp"
#include "Hooks/ValidateScripts.hpp"
#include "Hooks/gsmState_SessionActive.hpp"

namespace
{
std::unique_ptr<App> g_app;
}

App::App()
    : m_config(m_paths)
    , m_devConsole(m_config.GetDev())
{
    if (m_config.GetDev().waitForDebugger)
    {
        while (!IsDebuggerPresent())
        {
            std::this_thread::yield();
        }
    }

    AddSystem<LoggerSystem>(m_paths, m_config, m_devConsole);
    AddSystem<ScriptCompilationSystem>(m_paths);
    AddSystem<HookingSystem>();
    AddSystem<StateSystem>();
    AddSystem<PluginSystem>(m_config.GetPlugins(), m_paths);

    m_systems.shrink_to_fit();

    const auto filename = fmt::format(L"red4ext-{}.log", Utils::FormatCurrentTimestamp());

    auto logger = Utils::CreateLogger(L"RED4ext", filename, m_paths, m_config, m_devConsole);
    spdlog::set_default_logger(logger);

    spdlog::info("RED4ext (v{}) is initializing...", RED4EXT_VERSION_STR);

    spdlog::debug("Using the following paths:");
    spdlog::debug(L"  Root: {}", m_paths.GetRootDir());
    spdlog::debug(L"  RED4ext: {}", m_paths.GetRED4extDir());
    spdlog::debug(L"  Logs: {}", m_paths.GetLogsDir());
    spdlog::debug(L"  Config: {}", m_paths.GetConfigFile());
    spdlog::debug(L"  Plugins: {}", m_paths.GetPluginsDir());

    spdlog::debug("Using the following configuration:");
    spdlog::debug("  version: {}", m_config.GetVersion());

    const auto& dev = m_config.GetDev();
    spdlog::debug("  dev.console: {}", dev.hasConsole);

    const auto& loggingConfig = m_config.GetLogging();
    spdlog::debug("  logging.level: {}", spdlog::level::to_string_view(loggingConfig.level));
    spdlog::debug("  logging.flush_on: {}", spdlog::level::to_string_view(loggingConfig.flushOn));
    spdlog::debug("  logging.max_files: {}", loggingConfig.maxFiles);
    spdlog::debug("  logging.max_file_size: {} MB", loggingConfig.maxFileSize);

    const auto& pluginsConfig = m_config.GetPlugins();
    spdlog::debug("  plugins.enabled: {}", pluginsConfig.isEnabled);

    const auto& ignored = pluginsConfig.ignored;
    if (ignored.empty())
    {
        spdlog::debug("  plugins.ignored: []");
    }
    else
    {
        spdlog::debug(L"  plugins.ignored: [ {} ]", fmt::join(ignored, L", "));
    }

    spdlog::debug("Base address is: {}", reinterpret_cast<void*>(GetModuleHandle(nullptr)));

    const auto image = Image::Get();
    const auto& fileVer = image->GetFileVersion();

    const auto& productVer = image->GetProductVersion();
    spdlog::info("Product version: {}.{}{}", productVer.major, productVer.minor, productVer.patch);
    spdlog::info("File version: {}.{}.{}.{}", fileVer.major, fileVer.minor, fileVer.build, fileVer.revision);

    auto minimumVersion = RED4EXT_RUNTIME_2_31;
    if (fileVer < RED4EXT_RUNTIME_2_31)
    {
        spdlog::error(L"To use this version of RED4ext, ensure your game is updated to patch 2.31 or newer");
        return;
    }

    Addresses::Construct(m_paths);

    if (AttachHooks())
    {
        spdlog::info("RED4ext has been successfully initialized");
    }
    else
    {
        spdlog::error("RED4ext did not initialize properly");
    }
}

void App::Construct()
{
    g_app.reset(new App());
}

void App::Destruct()
{
    spdlog::info("RED4ext is terminating...");

    // Detaching hooks here and not in dtor, since the dtor can be called by CRT when the processes exists. We don't
    // really care if this will be called or not when the game exist ungracefully.

    spdlog::trace("Detaching the hooks...");

    DetourTransaction transaction;
    if (transaction.IsValid())
    {
        auto success = Hooks::CGameApplication::Detach() && Hooks::Main::Detach() && Hooks::ExecuteProcess::Detach() &&
                       Hooks::InitScripts::Detach() && Hooks::LoadScripts::Detach() &&
                       Hooks::ValidateScripts::Detach() && Hooks::AssertionFailed::Detach() &&
                       Hooks::gsmState_SessionActive::Detach();
        if (success)
        {
            transaction.Commit();
        }
    }

    g_app.reset(nullptr);
    spdlog::info("RED4ext has been terminated");

    spdlog::details::registry::instance().flush_all();
    spdlog::shutdown();
}

App* App::Get()
{
    return g_app.get();
}

void App::Startup()
{
    spdlog::info("RED4ext is starting up...");

    for (auto& system : m_systems)
    {
        system->Startup();
    }

    auto pluginNames = GetPluginSystem()->GetActivePlugins();
    GetLoggerSystem()->RotateLogs(pluginNames);

    spdlog::info("RED4ext has been started");
}

void App::Shutdown()
{
    spdlog::info("RED4ext is shutting down...");

    for (auto& system : m_systems | std::ranges::views::reverse)
    {
        system->Shutdown();
    }

    m_systems.clear();
    spdlog::info("RED4ext has been shut down");

    // Flushing the log here, since it is called in the main function, not when DLL is unloaded.
    spdlog::details::registry::instance().flush_all();
}

LoggerSystem* App::GetLoggerSystem()
{
    auto& system = m_systems.at(static_cast<size_t>(ESystemType::Logger));
    return static_cast<LoggerSystem*>(system.get());
}

HookingSystem* App::GetHookingSystem()
{
    auto& system = m_systems.at(static_cast<size_t>(ESystemType::Hooking));
    return static_cast<HookingSystem*>(system.get());
}

StateSystem* App::GetStateSystem()
{
    auto& system = m_systems.at(static_cast<size_t>(ESystemType::State));
    return static_cast<StateSystem*>(system.get());
}

PluginSystem* App::GetPluginSystem()
{
    auto& system = m_systems.at(static_cast<size_t>(ESystemType::Plugin));
    return static_cast<PluginSystem*>(system.get());
}

ScriptCompilationSystem* App::GetScriptCompilationSystem()
{
    auto& system = m_systems.at(static_cast<size_t>(ESystemType::Script));
    return static_cast<ScriptCompilationSystem*>(system.get());
}

const Paths* App::GetPaths() const
{
    return &m_paths;
}

bool App::AttachHooks() const
{
    spdlog::trace("Attaching hooks...");

    DetourTransaction transaction;
    if (!transaction.IsValid())
    {
        return false;
    }

    auto success = Hooks::Main::Attach() && Hooks::CGameApplication::Attach() && Hooks::ExecuteProcess::Attach() &&
                   Hooks::InitScripts::Attach() && Hooks::LoadScripts::Attach() && Hooks::ValidateScripts::Attach() &&
                   Hooks::AssertionFailed::Attach() && Hooks::CollectSaveableSystems::Attach() &&
                   Hooks::gsmState_SessionActive::Attach();
    if (success)
    {
        return transaction.Commit();
    }

    return false;
}
