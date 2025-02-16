#pragma once

#include <scc.h>

class ScriptCompilerFailure
{
    // Key used to prevent construction of this class outside of ScriptCompilerSettings.
    // This is necessary to construct the resulting std::variant in place.
    struct Key
    {
        explicit Key() = default;
    };

public:
    ScriptCompilerFailure(SccApi& aApi, SccResult* aResult, Key aKey);
    ScriptCompilerFailure(const ScriptCompilerFailure&) = delete;
    ScriptCompilerFailure& operator=(const ScriptCompilerFailure&) = delete;
    ~ScriptCompilerFailure();

    const std::string& GetMessage() const;

private:
    SccApi& m_scc;
    SccResult* m_result;
    std::string m_message;

    friend class ScriptCompilerSettings;
};
