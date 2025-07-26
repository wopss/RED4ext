#pragma once
#include "ScriptCompilerFailure.hpp"
#include "ScriptCompilerOutput.hpp"
#include <scc.h>

class ScriptCompilerSettings
{
public:
    using Result = std::variant<ScriptCompilerFailure, ScriptCompilerOutput>;

    ScriptCompilerSettings(SccApi& aApi, std::filesystem::path aR6Path);

    bool SupportsOutputCacheFileParameter() const;

    ScriptCompilerSettings* AddScriptPath(std::filesystem::path aPath);
    ScriptCompilerSettings* SetCustomCacheFile(std::filesystem::path aPath);
    ScriptCompilerSettings* SetOutputCacheFile(std::filesystem::path aPath);
    ScriptCompilerSettings* RegisterNeverRefType(std::string aType);
    ScriptCompilerSettings* RegisterMixedRefType(std::string aType);
    Result Compile();

private:
    SccApi& m_scc;
    std::filesystem::path m_r6Path;
    std::vector<std::filesystem::path> m_scriptPaths;
    std::filesystem::path m_customCacheFile;
    std::filesystem::path m_outputCacheFile;
    std::vector<std::string> m_neverRefTypes;
    std::vector<std::string> m_mixedRefTypes;
};
