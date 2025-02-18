#pragma once

#include <scc.h>

class ScriptCompilerSourceRef
{
public:
    uint8_t GetType() const;
    std::string_view GetName() const;
    std::string_view GetParentName() const;
    std::string_view GetPath() const;
    size_t GetLine() const;
    bool IsNative() const;

private:
    ScriptCompilerSourceRef(SccApi& aApi, SccOutput* aOutput, SccSourceRef* aSourceRef);

    SccApi& m_scc;
    SccOutput* m_output;
    SccSourceRef* m_sourceRef;

    friend class ScriptCompilerOutput;
};
