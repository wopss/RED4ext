#include "ScriptCompilerSettings.hpp"

ScriptCompilerSettings::ScriptCompilerSettings(SccApi& aApi, std::filesystem::path aR6Path)
    : m_scc(aApi)
    , m_r6Path(aR6Path)
{
}

bool ScriptCompilerSettings::SupportsOutputCacheFileParameter() const
{
    // added in redscript 0.5.18, previous versions will have this set to NULL
    return m_scc.settings_set_output_cache_file != nullptr;
}

ScriptCompilerSettings* ScriptCompilerSettings::AddScriptPath(std::filesystem::path aPath)
{
    m_scriptPaths.emplace_back(aPath);
    return this;
}

ScriptCompilerSettings* ScriptCompilerSettings::SetCustomCacheFile(std::filesystem::path aPath)
{
    m_customCacheFile = aPath;
    return this;
}

ScriptCompilerSettings* ScriptCompilerSettings::SetOutputCacheFile(std::filesystem::path aPath)
{
    m_outputCacheFile = aPath;
    return this;
}

ScriptCompilerSettings* ScriptCompilerSettings::RegisterNeverRefType(std::string aType)
{
    m_neverRefTypes.emplace_back(std::move(aType));
    return this;
}

ScriptCompilerSettings* ScriptCompilerSettings::RegisterMixedRefType(std::string aType)
{
    m_mixedRefTypes.emplace_back(std::move(aType));
    return this;
}

ScriptCompilerSettings::Result ScriptCompilerSettings::Compile()
{
    auto r6PathStr = m_r6Path.u8string();
    const auto settings = m_scc.settings_new(reinterpret_cast<const char*>(r6PathStr.c_str()));

    if (!m_customCacheFile.empty())
    {
        auto customCacheFileStr = m_customCacheFile.u8string();
        m_scc.settings_set_custom_cache_file(settings, reinterpret_cast<const char*>(customCacheFileStr.c_str()));
    }

    if (SupportsOutputCacheFileParameter() && !m_outputCacheFile.empty())
    {
        auto outputCacheFileStr = m_outputCacheFile.u8string();
        m_scc.settings_set_output_cache_file(settings, reinterpret_cast<const char*>(outputCacheFileStr.c_str()));
    }

    for (const auto& path : m_scriptPaths)
    {
        auto pathStr = path.u8string();
        m_scc.settings_add_script_path(settings, reinterpret_cast<const char*>(pathStr.c_str()));
    }

    const auto registerNeverRefType = m_scc.settings_register_never_ref_type;
    // Only configure never ref types when the compiler supports it.
    if (registerNeverRefType)
    {
        for (const auto& type : m_neverRefTypes)
        {
            auto typeStr = type.c_str();
            registerNeverRefType(settings, typeStr);
        }
    }

    const auto registerMixedRefType = m_scc.settings_register_mixed_ref_type;
    // Only configure mixed ref types when the compiler supports it.
    if (registerMixedRefType)
    {
        for (const auto& type : m_mixedRefTypes)
        {
            auto typeStr = type.c_str();
            registerMixedRefType(settings, typeStr);
        }
    }

    auto result = m_scc.compile(settings);

    if (auto output = m_scc.get_success(result))
    {
        return ScriptCompilerSettings::Result(std::in_place_type<ScriptCompilerOutput>, m_scc, result, output,
                                              ScriptCompilerOutput::Key());
    }
    else
    {
        return ScriptCompilerSettings::Result(std::in_place_type<ScriptCompilerFailure>, m_scc, result,
                                              ScriptCompilerFailure::Key());
    }
}
