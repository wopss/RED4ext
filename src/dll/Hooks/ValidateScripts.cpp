#include "ValidateScripts.hpp"
#include "Addresses.hpp"
#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "RED4ext/Scripting/ScriptReport.hpp"
#include "Systems/ScriptCompilationSystem.hpp"

namespace
{
bool isAttached = false;

bool _ScriptValidator_Validate(uint64_t self, uint64_t a1, RED4ext::ScriptReport& aReport);
Hook<decltype(&_ScriptValidator_Validate)> ScriptValidator_Validate(Hashes::ScriptValidator_Validate,
                                                                    &_ScriptValidator_Validate);

bool _ScriptValidator_Validate(uint64_t self, uint64_t a1, RED4ext::ScriptReport& aReport)
{
    aReport.fillErrors = true;
    const auto result = ScriptValidator_Validate(self, a1, aReport);
    std::vector<ValidationError> validationErrors;

    for (std::uint32_t i = 0; i < std::max(aReport.errors->size, 1u) - 1; ++i)
    {
        auto message = aReport.errors->entries[i].c_str();
        const auto error = ValidationError::FromString(message);
        validationErrors.push_back(error);

        auto ref = error.GetSourceRef();
        if (ref)
        {
            spdlog::error("Script validation error: {} at {}:{}", message, ref->file, ref->line + 1);
        }
        else
        {
            spdlog::error("Script validation error: {}", message);
        }
    }

    const auto& incompatiblePlugins = App::Get()->GetPluginSystem()->GetIncompatiblePlugins();
    const auto message = WritePopupMessage(validationErrors, incompatiblePlugins);
    if (message)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"{}", message.value());
    }

    return result;
}
} // namespace

bool Hooks::ValidateScripts::Attach()
{
    spdlog::trace("Trying to attach the hook for validate scripts at {:#x}...", ScriptValidator_Validate.GetAddress());

    auto result = ScriptValidator_Validate.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for validate scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for validate scripts was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::ValidateScripts::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for validate scripts at {:#x}...", ScriptValidator_Validate.GetAddress());

    auto result = ScriptValidator_Validate.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for validate scripts. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for validate scripts was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}

std::optional<std::wstring> WritePopupMessage(const std::vector<ValidationError>& validationErrors,
                                              const std::vector<PluginSystem::PluginName>& incompatiblePlugins)
{
    if (validationErrors.empty())
    {
        return {};
    }

    std::unordered_set<std::string_view> faultyScriptFiles;

    for (const auto& error : validationErrors)
    {
        if (const auto ref = error.GetSourceRef())
        {
            faultyScriptFiles.insert(ref->file);
        }
    }

    if (faultyScriptFiles.empty() && incompatiblePlugins.empty())
    {
        return L"Script validation has failed for an unknown reason. "
               L"You should verify game files with Steam/GOG and retry.";
    }

    fmt::wmemory_buffer message;
    auto outIt = std::back_inserter(message);

    if (!incompatiblePlugins.empty())
    {
        fmt::format_to(outIt, L"The following RED4ext plugins could not be loaded because they are "
                              L"incompatible with the current version of the game:\n");

        for (const auto& plugin : incompatiblePlugins)
        {
            fmt::format_to(outIt, L"- {}\n", plugin);
        }
        fmt::format_to(outIt, L"\n");
    }

    if (!faultyScriptFiles.empty())
    {
        fmt::format_to(outIt, L"The following scripts contain invalid native definitions and will prevent "
                              L"your game from starting:\n");

        for (const auto& file : faultyScriptFiles)
        {
            fmt::format_to(outIt, L"- {}\n", Utils::Widen(file));
        }
        fmt::format_to(outIt, L"\n");
    }

    fmt::format_to(outIt, L"Check if these mods are up-to-date and installed correctly. If you keep seeing "
                          L"this message after updating/re-installing them, you might have to remove them "
                          L"in order to play the game.\n"
                          L"More details can be found in the logs.\n");

    return std::wstring(message.data(), message.size());
}
