#include "Funcs.hpp"

#include "App.hpp"
#include "Utils.hpp"

namespace
{
std::shared_ptr<PluginBase> GetPluginByHandle(RED4ext::PluginHandle aHandle)
{
    auto app = App::Get();
    if (!app)
    {
        return nullptr;
    }

    const auto module = std::bit_cast<HMODULE>(aHandle);
    auto pluginSystem = app->GetPluginSystem();
    auto plugin = pluginSystem->GetPlugin(module);
    if (!plugin)
    {
        spdlog::warn("Could not find a plugin with handle {}", fmt::ptr(module));
        return nullptr;
    }

    return plugin;
}
} // namespace

namespace v1
{
bool Hooking::Attach(RED4ext::PluginHandle aHandle, void* aTarget, void* aDetour, void** aOriginal)
{
    const auto module = std::bit_cast<HMODULE>(aHandle);
    spdlog::trace("Attach request received from plugin with handle {}", fmt::ptr(module));

    if (aTarget == nullptr || aDetour == nullptr)
    {
        spdlog::warn("One of the required parameters for attaching hook is NULL");
        return false;
    }

    auto app = App::Get();
    if (!app)
    {
        return false;
    }

    auto plugin = GetPluginByHandle(aHandle);
    if (!plugin)
    {
        return false;
    }

    auto hookingSystem = app->GetHookingSystem();
    return hookingSystem->Attach(plugin, aTarget, aDetour, aOriginal);
}

bool Hooking::Detach(RED4ext::PluginHandle aHandle, void* aTarget)
{
    const auto module = std::bit_cast<HMODULE>(aHandle);
    spdlog::trace("Detach request received from plugin with handle {}", fmt::ptr(module));

    if (aTarget == nullptr)
    {
        spdlog::warn("One of the required parameters for detaching hook is NULL");
        return false;
    }

    auto app = App::Get();
    if (!app)
    {
        return false;
    }

    auto plugin = GetPluginByHandle(aHandle);
    if (!plugin)
    {
        return false;
    }

    auto hookingSystem = app->GetHookingSystem();
    return hookingSystem->Detach(plugin, aTarget);
}

bool GameStates::AddHook(RED4ext::PluginHandle aHandle, RED4ext::EGameStateType aType, const RED4ext::GameState& aState)
{
    const auto module = std::bit_cast<HMODULE>(aHandle);
    spdlog::trace("Request to add a game state hook has been received from plugin with handle {}", fmt::ptr(module));

    auto app = App::Get();
    if (!app)
    {
        return false;
    }

    auto plugin = GetPluginByHandle(aHandle);
    if (!plugin)
    {
        return false;
    }

    if (aType < RED4ext::EGameStateType::BaseInitialization || aType > RED4ext::EGameStateType::Shutdown)
    {
        return false;
    }

    auto stateSystem = app->GetStateSystem();
    return stateSystem->AddHook(plugin, aType, aState);
}

bool Scripts::Add(RED4ext::PluginHandle aHandle, const wchar_t* aPath)
{
    auto app = App::Get();
    if (!app)
    {
        return false;
    }

    auto plugin = GetPluginByHandle(aHandle);
    if (!plugin)
    {
        return false;
    }

    auto scriptCompilationSystem = app->GetScriptCompilationSystem();
    return scriptCompilationSystem->Add(plugin, aPath);
}
} // namespace v1
