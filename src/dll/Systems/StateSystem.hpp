#pragma once

#include "ISystem.hpp"

#include "PluginBase.hpp"

class StateSystem final : public ISystem
{
public:
    ESystemType GetType() override;

    void Startup() override;
    void Shutdown() override;

    bool OnChangeState(RED4ext::CGameApplication& aApp, RED4ext::EGameStateType aNextState);
    bool OnDoState(RED4ext::CGameApplication& aApp);

    bool AddHook(std::shared_ptr<PluginBase> aPlugin, RED4ext::EGameStateType aStateType,
                 const RED4ext::GameState& aState);

private:
    struct StateHooks
    {
        struct Action
        {
            struct Hook
            {
                std::shared_ptr<PluginBase> plugin;
                RED4ext::EGameStateResult (*hook)(RED4ext::CGameApplication&);
            };

            void Shutdown();

            std::wstring_view name;
            std::vector<Hook> onBefore;
            std::vector<Hook> onAfter;
        };

        void Shutdown();

        Action& GetActionHooksForStatus(RED4ext::EGameStateStatus aStateStatus);

        std::wstring_view name;
        Action onEnter = {.name = L"Enter"};
        Action onTick = {.name = L"Tick"};
        Action onExit = {.name = L"Exit"};
    };

    StateHooks& GetStateHooks(RED4ext::EGameStateType aStateType);

    StateHooks m_baseInitialization = {.name = L"BaseInitialization"};
    StateHooks m_initialization = {.name = L"Initialization"};
    StateHooks m_running = {.name = L"Running"};
    StateHooks m_shutdown = {.name = L"Shutdown"};
    RED4ext::EGameStateStatus m_nextStateStatus = RED4ext::EGameStateStatus::NextState;
    bool m_originalStateActionFinished = false;
    bool m_shouldSwitchState = false;
};
