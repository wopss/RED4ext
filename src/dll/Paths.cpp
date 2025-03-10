#include "Paths.hpp"

#include "Utils.hpp"

Paths::Paths()
{
    std::wstring fileName;
    auto hr = wil::GetModuleFileNameW(nullptr, fileName);
    if (FAILED(hr))
    {
        SHOW_LAST_ERROR_MESSAGE_AND_EXIT_FILE_LINE(L"Could not get game's file name.");
        return;
    }

    m_exe = fileName;
    m_root = m_exe
                 .parent_path()  // Resolve to "x64" directory.
                 .parent_path()  // Resolve to "bin" directory.
                 .parent_path(); // Resolve to game's root directory.
}

std::filesystem::path Paths::GetRootDir() const
{
    return m_root;
}

std::filesystem::path Paths::GetX64Dir() const
{
    return GetRootDir() / L"bin" / L"x64";
}

std::filesystem::path Paths::GetExe() const
{
    return m_exe;
}

std::filesystem::path Paths::GetRED4extDir() const
{
    return GetRootDir() / L"red4ext";
}

std::filesystem::path Paths::GetLogsDir() const
{
    return GetRED4extDir() / L"logs";
}

std::filesystem::path Paths::GetPluginsDir() const
{
    return GetRED4extDir() / L"plugins";
}

std::filesystem::path Paths::GetRedscriptPathsFile() const
{
    return GetRED4extDir() / L"redscript_paths.txt";
}

std::filesystem::path Paths::GetR6Scripts() const
{
    return GetRootDir() / L"r6" / L"scripts";
}

std::filesystem::path Paths::GetDefaultScriptsBlob() const
{
    return GetRootDir() / L"r6" / L"cache" / "final.redscripts";
}

std::filesystem::path Paths::GetR6CacheModded() const
{
    return GetRootDir() / L"r6" / L"cache" / L"modded";
}

std::filesystem::path Paths::GetR6Dir() const
{
    return GetRootDir() / L"r6";
}

const std::filesystem::path Paths::GetConfigFile() const
{
    return GetRED4extDir() / L"config.ini";
}
