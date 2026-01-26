#include "Utils.hpp"
#include "Config.hpp"
#include "DevConsole.hpp"
#include "Paths.hpp"

#include <ctime>
#include <cwctype>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Utils::CreateLogger(const std::wstring_view aLogName, const std::wstring_view aFilename,
                                                    const Paths& aPaths, const Config& aConfig,
                                                    const DevConsole& aDevConsole)
{
    try
    {
        auto dir = aPaths.GetLogsDir();

        std::error_code err;
        auto exists = std::filesystem::exists(dir, err);
        if (err)
        {
            auto errVal = err.value();
            const auto& category = err.category();
            auto msg = category.message(errVal);

            SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(
                L"An error occurred while checking logs directory existence:\n{}\n\nDirectory: {}", Utils::Widen(msg),
                dir);

            return nullptr;
        }

        if (!exists)
        {
            std::filesystem::create_directories(dir, err);
            if (err)
            {
                auto errVal = err.value();
                const auto& category = err.category();
                auto msg = category.message(errVal);

                SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(
                    L"An error occurred while creating the logs directory:\n{}\n\nDirectory: {}", Utils::Widen(msg),
                    dir);

                return nullptr;
            }
        }

        constexpr auto oneByte = 1;
        constexpr auto oneKbInB = 1024 * oneByte;
        constexpr auto oneMbInB = 1024 * oneKbInB;

        const auto& loggingConfig = aConfig.GetLogging();
        const size_t maxFiles = loggingConfig.maxFiles;
        const size_t maxFileSize = static_cast<size_t>(loggingConfig.maxFileSize) * oneMbInB;

        const std::string logName = Narrow(aLogName);
        std::vector<spdlog::sink_ptr> sinks;

        auto file = dir / aFilename;
        sinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file, maxFileSize, maxFiles, true));

        const auto& dev = aConfig.GetDev();
        if (dev.hasConsole && aDevConsole.IsOutputRedirected())
        {
            sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        }

        auto logger = std::make_shared<spdlog::logger>(logName, sinks.begin(), sinks.end());

        logger->set_level(loggingConfig.level);
        logger->flush_on(loggingConfig.flushOn);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] [%6t] [%n] %v");

        spdlog::register_logger(logger);

        return logger;
    }
    catch (const std::exception& e)
    {
        std::string_view msg = e.what();
        if (msg.starts_with("rotating_file_sink: failed renaming"))
        {
            SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(
                L"Unable to rotate the log file. Please ensure that the game is completely closed and not running in "
                L"the background.\n\n{}",
                Utils::Widen(msg));
        }
        else
        {
            SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"An exception occurred while creating the logger:\n{}",
                                                Utils::Widen(msg));
        }
    }

    return nullptr;
}

std::wstring Utils::GetStateName(RED4ext::EGameStateType aStateType)
{
    using enum RED4ext::EGameStateType;
    switch (aStateType)
    {
    case BaseInitialization:
    {
        return L"BaseInitialization";
    }
    case Initialization:
    {
        return L"Initialization";
    }
    case Running:
    {
        return L"Running";
    }
    case Shutdown:
    {
        return L"Shutdown";
    }
    default:
    {
        return L"unknown";
    }
    }
}

std::wstring Utils::FormatSystemMessage(uint32_t aMessageId)
{
    wil::last_error_context last_error;
    wil::unique_hlocal_ptr<wchar_t> buffer;

    auto len =
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      nullptr, aMessageId, LANG_USER_DEFAULT, wil::out_param_ptr<LPWSTR>(buffer), 0, nullptr);
    if (!len)
    {
        return fmt::format(L"Could not format the system message for the specified message id ({}), error code: {}",
                           aMessageId, GetLastError());
    }

    std::wstring_view res = buffer.get();

    // Remove the new lines at the end of the message, they are annoying.
    if (res.ends_with(L'\n'))
    {
        res.remove_suffix(1);
    }

    if (res.ends_with(L'\r'))
    {
        res.remove_suffix(1);
    }

    return std::wstring(res);
}

std::wstring Utils::FormatLastError()
{
    auto err = GetLastError();
    return FormatSystemMessage(err);
}

std::wstring Utils::FormatCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // Convert to std::tm for formatting
    std::tm now_tm;
    localtime_s(&now_tm, &now_c);

    return fmt::format(L"{:04d}-{:02d}-{:02d}-{:02d}-{:02d}-{:02d}", now_tm.tm_year + 1900, now_tm.tm_mon + 1,
                       now_tm.tm_mday, now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec);
}

int32_t Utils::ShowMessageBoxEx(const std::wstring_view aCaption, const std::wstring_view aText, uint32_t aType)
{
    return MessageBox(nullptr, aText.data(), aCaption.data(), aType);
}

int32_t Utils::ShowMessageBox(const std::wstring_view aText, uint32_t aType)
{
    return ShowMessageBoxEx(L"RED4ext", aText, aType);
}

std::string Utils::Narrow(const std::wstring_view aText)
{
    if (aText.empty())
    {
        return "";
    }

    std::string result;

    auto len =
        WideCharToMultiByte(CP_UTF8, 0, aText.data(), static_cast<int32_t>(aText.size()), nullptr, 0, NULL, NULL);
    if (len)
    {
        result.resize(len);
        len = WideCharToMultiByte(CP_UTF8, 0, aText.data(), static_cast<int32_t>(aText.size()), result.data(),
                                  static_cast<int32_t>(result.size()), NULL, NULL);
    }

    // Second pass.
    if (len <= 0)
    {
        result = fmt::format("Failed to convert wide to narrow string, last error is {}", GetLastError());
    }

    return result;
}

std::wstring Utils::Widen(const std::string_view aText)
{
    if (aText.empty())
    {
        return L"";
    }

    std::wstring result;

    auto len = MultiByteToWideChar(CP_UTF8, 0, aText.data(), static_cast<int32_t>(aText.size()), nullptr, 0);
    if (len)
    {
        result.resize(len);
        len = MultiByteToWideChar(CP_UTF8, 0, aText.data(), static_cast<int32_t>(aText.size()), result.data(),
                                  static_cast<int32_t>(result.size()));
    }

    // Second pass.
    if (len <= 0)
    {
        result = fmt::format(L"Failed to convert narrow to wide string, last error is {}", GetLastError());
    }

    return result;
}

std::wstring Utils::ToLower(const std::wstring& acText)
{
    std::wstring text = acText;

    std::transform(text.begin(), text.end(), text.begin(), std::towlower);
    return text;
}
