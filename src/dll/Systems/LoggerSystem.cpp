#include "LoggerSystem.hpp"

#include "Config.hpp"
#include "Paths.hpp"

LoggerSystem::LoggerSystem(const Paths& aPaths, const Config& aConfig, const DevConsole& aDevConsole)
    : m_paths(aPaths)
    , m_config(aConfig)
    , m_devConsole(aDevConsole)
{
}

ESystemType LoggerSystem::GetType()
{
    return ESystemType::Logger;
}

void LoggerSystem::Startup()
{
}

void LoggerSystem::Shutdown()
{
    auto count = m_loggers.size();

    spdlog::trace("Flusing {} logger(s)...", count);

    for (auto& [plugin, logger] : m_loggers)
    {
        logger->flush();
    }

    m_loggers.clear();
    spdlog::trace("{} logger(s) flushed", count);
}

void LoggerSystem::RotateLogs(std::vector<std::wstring> pluginNames) const
{
    std::error_code error;
    auto files = std::filesystem::directory_iterator(m_paths.GetLogsDir(), error);

    if (error)
    {
        // Ignore and try the next time.
        return;
    }
    spdlog::trace("Rotate logs...");
    pluginNames.emplace_back(L"RED4ext");

    // List all log files.
    std::vector<std::filesystem::path> logs;

    for (const std::filesystem::directory_entry& file : files)
    {
        const auto fileName = file.path().filename().wstring();

        if (file.is_regular_file() && fileName.ends_with(L".log"))
        {
            logs.emplace_back(file.path());
        }
    }
    const Config::LoggingConfig config = m_config.GetLogging();

    // Rotate oldest logs per plugin.
    for (auto pluginName : pluginNames)
    {
        pluginName = Utils::ToLower(pluginName);
        std::vector<std::filesystem::path> pluginLogs;

        logs.erase(std::remove_if(logs.begin(), logs.end(),
                                  [&pluginName, &pluginLogs](const auto& log)
                                  {
                                      if (log.filename().wstring().starts_with(pluginName))
                                      {
                                          pluginLogs.push_back(log);
                                          return true;
                                      }
                                      return false;
                                  }),
                   logs.end());
        std::sort(pluginLogs.begin(), pluginLogs.end(), [](const auto& a, const auto& b) { return a > b; });

        // maxFiles is guaranteed to be >= 1.
        for (size_t i = config.maxFiles; i < pluginLogs.size(); i++)
        {
            std::filesystem::remove(pluginLogs[i], error);
        }
    }
    // Remove log files of removed plugins.
    for (const auto& log : logs)
    {
        std::filesystem::remove(log, error);
    }
}

void LoggerSystem::Trace(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::trace, aText);
}

void LoggerSystem::Trace(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::trace, aText);
}

void LoggerSystem::Debug(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::debug, aText);
}

void LoggerSystem::Debug(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::debug, aText);
}

void LoggerSystem::Info(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::info, aText);
}

void LoggerSystem::Info(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::info, aText);
}

void LoggerSystem::Warn(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::warn, aText);
}

void LoggerSystem::Warn(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::warn, aText);
}

void LoggerSystem::Error(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::err, aText);
}

void LoggerSystem::Error(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::err, aText);
}

void LoggerSystem::Critical(std::shared_ptr<PluginBase> aPlugin, std::string_view aText)
{
    Log(aPlugin, spdlog::level::critical, aText);
}

void LoggerSystem::Critical(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText)
{
    Log(aPlugin, spdlog::level::critical, aText);
}
