#pragma once

#include "Hook.hpp"
#include "ISystem.hpp"
#include "Paths.hpp"
#include "PluginBase.hpp"
#include "SourceRefRepository.hpp"

struct FixedWString
{
    uint32_t length;
    uint32_t maxLength;
    const wchar_t* str;
};

class ScriptCompilationSystem final : public ISystem
{
    using Map_t = std::unordered_multimap<std::shared_ptr<PluginBase>, std::filesystem::path>;
    using MapIter_t = Map_t::iterator;

public:
    ScriptCompilationSystem(const Paths& aPaths);

    ESystemType GetType() override;

    void Startup() override;
    void Shutdown() override;

    bool Add(std::shared_ptr<PluginBase> aPlugin, const wchar_t* aPath);

    void SetScriptsBlob(const std::filesystem::path& aPath);
    const std::filesystem::path& GetScriptsBlob() const;
    bool HasScriptsBlob() const;

    void SetModdedScriptsBlob(const std::filesystem::path& aPath);
    const std::filesystem::path& GetModdedScriptsBlob() const;
    bool HasModdedScriptsBlob() const;

    std::wstring GetCompilationArgs(const FixedWString& aOriginal);
    const Map_t& GetScriptPaths() const;

    SourceRefRepository& GetSourceRefRepository();

private:
    void Add(std::shared_ptr<PluginBase> aPlugin, std::filesystem::path path);

    const Paths& m_paths;

    std::mutex m_mutex;
    Map_t m_scriptPaths;
    bool m_hasScriptsBlob;
    std::filesystem::path m_scriptsBlobPath;
    bool m_hasModdedScriptsBlob;
    std::filesystem::path m_moddedScriptsBlobPath;
    SourceRefRepository m_sourceRefs;
};
