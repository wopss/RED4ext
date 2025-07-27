#include "ScriptCompilationSystem.hpp"
#include "Utils.hpp"

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
    m_scriptsBlobPath = aPath;
    m_hasScriptsBlob = true;
}

const std::filesystem::path& ScriptCompilationSystem::GetScriptsBlob() const
{
    return m_scriptsBlobPath;
}

bool ScriptCompilationSystem::HasScriptsBlob() const
{
    return m_hasScriptsBlob;
}

void ScriptCompilationSystem::SetModdedScriptsBlob(const std::filesystem::path& aPath)
{
    m_moddedScriptsBlobPath = aPath;
    m_hasModdedScriptsBlob = true;
}

const std::filesystem::path& ScriptCompilationSystem::GetModdedScriptsBlob() const
{
    return m_moddedScriptsBlobPath;
}

bool ScriptCompilationSystem::HasModdedScriptsBlob() const
{
    return m_hasModdedScriptsBlob;
}

void ScriptCompilationSystem::RegisterNeverRefType(std::string aType)
{
    m_neverRefTypes.emplace_back(std::move(aType));
}

const std::vector<std::string>& ScriptCompilationSystem::GetNeverRefTypes() const
{
    return m_neverRefTypes;
}

void ScriptCompilationSystem::RegisterMixedRefType(std::string aType)
{
    m_mixedRefTypes.emplace_back(std::move(aType));
}

const std::vector<std::string>& ScriptCompilationSystem::GetMixedRefTypes() const
{
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
    fmt::wmemory_buffer buffer;
    if (m_hasScriptsBlob)
    {
        spdlog::info("Using scriptsBlobPath");
        format_to(std::back_inserter(buffer), LR"(-compile "{}" "{}")", m_paths.GetR6Scripts(), m_scriptsBlobPath);
    }
    else
    {
        format_to(std::back_inserter(buffer), aOriginal.str);
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
    format_to(std::back_inserter(buffer), LR"( -compilePathsFile "{}")", pathsFilePath);
    return fmt::to_string(buffer);
}

const ScriptCompilationSystem::Map_t& ScriptCompilationSystem::GetScriptPaths() const
{
    return m_scriptPaths;
}

SourceRefRepository& ScriptCompilationSystem::GetSourceRefRepository()
{
    return m_sourceRefs;
}
