#include "ScriptCompilationSystem.hpp"
#include "ESystemType.hpp"
#include "Paths.hpp"
#include "PluginBase.hpp"
#include "SourceRefRepository.hpp"
#include "Utils.hpp"

#include <fmt/format.h>
#include <fmt/xchar.h>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

ScriptCompilationSystem::ScriptCompilationSystem(const Paths& aPaths)
    : m_paths(aPaths)
    , m_hasScriptsBlob(false)
    , m_hasModdedScriptsBlob(false)
{
}

ESystemType ScriptCompilationSystem::GetType()
{
    return ESystemType::Script;
}

void ScriptCompilationSystem::Startup()
{
}

void ScriptCompilationSystem::Shutdown()
{
}

void ScriptCompilationSystem::Add(std::shared_ptr<PluginBase> aPlugin, std::filesystem::path aPath)
{
    std::scoped_lock _(m_mutex);
    m_scriptPaths.emplace(aPlugin, std::move(aPath));
}

void ScriptCompilationSystem::SetScriptsBlob(const std::filesystem::path& aPath)
{
    std::scoped_lock _(m_mutex);
    m_scriptsBlobPath = aPath;
    m_hasScriptsBlob = true;
}

std::filesystem::path ScriptCompilationSystem::GetScriptsBlob() const
{
    std::scoped_lock _(m_mutex);
    return m_scriptsBlobPath;
}

bool ScriptCompilationSystem::HasScriptsBlob() const
{
    std::scoped_lock _(m_mutex);
    return m_hasScriptsBlob;
}

void ScriptCompilationSystem::SetModdedScriptsBlob(const std::filesystem::path& aPath)
{
    std::scoped_lock _(m_mutex);
    m_moddedScriptsBlobPath = aPath;
    m_hasModdedScriptsBlob = true;
}

std::filesystem::path ScriptCompilationSystem::GetModdedScriptsBlob() const
{
    std::scoped_lock _(m_mutex);
    return m_moddedScriptsBlobPath;
}

bool ScriptCompilationSystem::HasModdedScriptsBlob() const
{
    std::scoped_lock _(m_mutex);
    return m_hasModdedScriptsBlob;
}

void ScriptCompilationSystem::RegisterNeverRefType(std::string aType)
{
    std::scoped_lock _(m_mutex);
    m_neverRefTypes.emplace_back(std::move(aType));
}

std::vector<std::string> ScriptCompilationSystem::GetNeverRefTypes() const
{
    std::scoped_lock _(m_mutex);
    return m_neverRefTypes;
}

void ScriptCompilationSystem::RegisterMixedRefType(std::string aType)
{
    std::scoped_lock _(m_mutex);
    m_mixedRefTypes.emplace_back(std::move(aType));
}

std::vector<std::string> ScriptCompilationSystem::GetMixedRefTypes() const
{
    std::scoped_lock _(m_mutex);
    return m_mixedRefTypes;
}

bool ScriptCompilationSystem::Add(std::shared_ptr<PluginBase> aPlugin, const wchar_t* aPath)
{
    spdlog::trace(L"Adding path to script compilation: '{}'", aPath);
    auto resolvedPath = std::filesystem::path(aPath);
    if (resolvedPath.is_absolute())
    {
        if (std::filesystem::exists(resolvedPath))
        {
            spdlog::trace(L"Found absolute path: {}", resolvedPath.wstring().c_str());
            Add(aPlugin, resolvedPath);
            return true;
        }
        else
        {
            spdlog::error(L"Could not find absolute path: {}", resolvedPath.wstring().c_str());
            return false;
        }
    }
    else
    {
        resolvedPath = aPlugin->GetPath().parent_path() / aPath;
        if (std::filesystem::exists(resolvedPath))
        {
            spdlog::trace(L"Found path relative to plugin: {}", resolvedPath.wstring().c_str());
            Add(aPlugin, resolvedPath);
            return true;
        }
        else
        {
            spdlog::error(L"Could not find path relative to plugin: {}", resolvedPath.wstring().c_str());
            return false;
        }
    }
}

std::wstring ScriptCompilationSystem::GetCompilationArgs(const FixedWString& aOriginal)
{
    std::scoped_lock _(m_mutex);

    fmt::wmemory_buffer buffer;

    if (m_hasScriptsBlob)
    {
        spdlog::info("Using scriptsBlobPath");
        fmt::format_to(std::back_inserter(buffer), LR"(-compile "{}" "{}")", m_paths.GetR6Scripts(), m_scriptsBlobPath);
    }
    else
    {
        fmt::format_to(std::back_inserter(buffer), L"{}", aOriginal.str);
    }

    spdlog::info("Adding paths to redscript compilation:");

    auto pathsFilePath = m_paths.GetRedscriptPathsFile();
    std::wofstream pathsFile(pathsFilePath, std::ios::out);

    for (const auto& [plugin, path] : m_scriptPaths)
    {
        spdlog::info(L"{}: '{}'", plugin->GetName(), path);
        pathsFile << path.wstring() << std::endl;
    }

    spdlog::info(L"Paths written to: '{}'", pathsFilePath);
    fmt::format_to(std::back_inserter(buffer), LR"( -compilePathsFile "{}")", pathsFilePath);

    return fmt::to_wstring(buffer);
}

ScriptCompilationSystem::Map_t ScriptCompilationSystem::GetScriptPaths() const
{
    std::scoped_lock _(m_mutex);
    return m_scriptPaths;
}

SourceRefRepository& ScriptCompilationSystem::GetSourceRefRepository()
{
    return m_sourceRefs;
}
