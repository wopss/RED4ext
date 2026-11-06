#include "ScriptCompilerFailure.hpp"

ScriptCompilerFailure::ScriptCompilerFailure(SccApi& aApi, SccResult* aResult, Key aKey)
    : m_scc(aApi)
    , m_result(aResult)
{
    RED4EXT_UNUSED_PARAMETER(aKey);

    char buffer[256] = {0};
    spdlog::trace("m_scc.copy_error");
    m_scc.copy_error(aResult, buffer, sizeof(buffer));
    spdlog::trace("m_scc.copy_error -> done");

    std::string errorMessage(buffer);

    // truncate to first line to keep it short
    auto lineEnd = errorMessage.find('\n');
    if (lineEnd != std::string::npos)
    {
        errorMessage.resize(lineEnd);
    }

    // replace trailing characters with ellipsis
    if (errorMessage.length() > sizeof(buffer) - 4)
    {
        errorMessage.resize(sizeof(buffer) - 4);
        errorMessage.append("...");
    }

    m_message = errorMessage;
}

ScriptCompilerFailure::~ScriptCompilerFailure()
{
    m_scc.free_result(m_result);
}

const std::string& ScriptCompilerFailure::GetMessage() const
{
    return m_message;
}
