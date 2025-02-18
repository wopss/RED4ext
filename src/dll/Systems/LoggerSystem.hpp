#pragma once

#include "ISystem.hpp"
#include "PluginBase.hpp"
#include "Utils.hpp"

class LoggerSystem final : public ISystem
{
public:
    LoggerSystem(const Paths& aPaths, const Config& aConfig, const DevConsole& aDevConsole);

    ESystemType GetType() override;

    void Startup() override;
    void Shutdown() override;

    void RotateLogs(std::vector<std::wstring> plugins) const;

    void Trace(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Trace(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

    void Debug(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Debug(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

    void Info(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Info(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

    void Warn(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Warn(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

    void Error(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Error(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

    void Critical(std::shared_ptr<PluginBase> aPlugin, std::string_view aText);
    void Critical(std::shared_ptr<PluginBase> aPlugin, std::wstring_view aText);

private:
    template<typename T>
    void Log(std::shared_ptr<PluginBase> aPlugin, spdlog::level::level_enum aLevel, std::basic_string_view<T> aText)
    {
        if (!aPlugin)
        {
            return;
        }

        std::shared_ptr<spdlog::logger> logger;

        {
            std::scoped_lock _(m_loggersMutex);

            auto it = m_loggers.find(aPlugin);
            if (it != m_loggers.end())
            {
                logger = it->second;
            }
            else
            {
                const auto& path = aPlugin->GetPath();
                const auto stem = path.stem();
                auto fileName = stem.wstring();

                fileName = Utils::ToLower(fileName);

                const auto logName = aPlugin->GetName();

                fileName = fmt::format(L"{}-{}.log", fileName, Utils::FormatCurrentTimestamp());
                logger = Utils::CreateLogger(logName, fileName, m_paths, m_config, m_devConsole);
                m_loggers.emplace(aPlugin, logger);
            }
        }

        logger->log(aLevel, aText);
    }

    const Paths& m_paths;
    const Config& m_config;
    const DevConsole& m_devConsole;

    std::mutex m_loggersMutex;
    std::unordered_map<std::shared_ptr<PluginBase>, std::shared_ptr<spdlog::logger>> m_loggers;
};
