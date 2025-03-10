#pragma once

#include "ScriptCompilerSourceRef.hpp"

#include <scc.h>

class ScriptCompilerOutput
{
    // Key used to prevent construction of this class outside of ScriptCompilerSettings.
    // This is necessary to construct the resulting std::variant in place.
    struct Key
    {
        explicit Key() = default;
    };

public:
    ScriptCompilerOutput(SccApi& aApi, SccResult* aResult, SccOutput* aOutput, Key aKey);
    ScriptCompilerOutput(const ScriptCompilerOutput&) = delete;
    ScriptCompilerOutput& operator=(const ScriptCompilerOutput&) = delete;
    ~ScriptCompilerOutput();

    size_t GetSourceRefCount() const;
    ScriptCompilerSourceRef GetSourceRef(size_t aIndex) const;

private:
    SccApi& m_scc;
    SccResult* m_result;
    SccOutput* m_output;

    friend class ScriptCompilerSettings;
};
