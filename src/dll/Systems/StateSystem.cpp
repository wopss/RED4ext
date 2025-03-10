#include "StateSystem.hpp"

ESystemType StateSystem::GetType()
{
    return ESystemType::State;
}

void StateSystem::Startup()
{
}

void StateSystem::Shutdown()
{
    spdlog::trace("Removing all game state hooks...");

    m_baseInitialization.Shutdown();
    m_initialization.Shutdown();
    m_running.Shutdown();
    m_shutdown.Shutdown();

    spdlog::trace("All game state hooks were removed successfully");
}

bool StateSystem::OnChangeState(RED4ext::CGameApplication& aApp, const RED4ext::EGameStateType aNextState)
{
    const auto nextStateRef = aApp.states.Get(aNextState);
    if (!nextStateRef)
    {
        assert(false && "Unable to get next state reference from states map!");

        spdlog::error("Unable to get next state reference for state with ID {} from states map!",
                      static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aNextState));

        // Override failed.
        return false;
    }

    const auto nextState = *nextStateRef;
    if (nextState == aApp.nextState)
    {
        assert(aApp.nextState && "Next state was unexpectedly unset!");

        spdlog::trace("Already transitioned to state '{}' ({}), ignoring change of state.", aApp.nextState->GetName(),
                      static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aApp.nextState->GetType()));

        // Override successful.
        return true;
    }

    if (aApp.nextState)
    {
        spdlog::warn(
            "Already set up next state to '{}' ({}), ignoring change of state to '{}' ({}).", aApp.nextState->GetName(),
            static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aApp.nextState->GetType()),
            nextState->GetName(), static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(nextState->GetType()));
    }
    else
    {
        aApp.nextState = nextState;
    }

    m_shouldSwitchState = true;

    // Override successful.
    return true;
}

bool StateSystem::OnDoState(RED4ext::CGameApplication& aApp)
{
    const auto switchToNextState = aApp.stateStatus == RED4ext::EGameStateStatus::NextState;

    if (!aApp.currentState || switchToNextState)
    {
        if (aApp.nextState || switchToNextState)
        {
            aApp.currentState = aApp.nextState;
            aApp.stateStatus = RED4ext::EGameStateStatus::Enter;
            aApp.nextState = nullptr;
        }
        else
        {
            spdlog::error("No state to execute!");

            // Override failed.
            return false;
        }

        m_originalStateActionFinished = false;
        m_shouldSwitchState = false;

        // Override successful.
        return true;
    }

    assert(aApp.stateStatus != RED4ext::EGameStateStatus::NextState && "State status unexpectedly equal to NextState!");

    static const auto runHooks = [](std::wstring_view aAction, auto& aHooks, RED4ext::CGameApplication& aApp)
    {
        bool finished = true;

        for (auto it = aHooks.begin(); it != aHooks.end();)
        {
            const std::wstring_view pluginName = it->plugin ? it->plugin->GetName() : L"RED4ext";

            try
            {
                switch (const auto result = it->hook(aApp))
                {
                case RED4ext::EGameStateResult::Finished:
                    spdlog::trace(L"Finished running '{}' registered by '{}'", aAction, pluginName);
                    it = aHooks.erase(it);
                    break;
                case RED4ext::EGameStateResult::Running:
                    finished = false;
                    [[fallthrough]];
                case RED4ext::EGameStateResult::Observing:
                    ++it;
                    break;
                default:
                    spdlog::warn(L"Unrecognized result {} for '{}' registered by '{}' - assuming the hook finished",
                                 static_cast<std::underlying_type_t<RED4ext::EGameStateResult>>(result), aAction,
                                 pluginName);
                    it = aHooks.erase(it);
                    break;
                }
            }
            catch (const std::exception& e)
            {
                spdlog::warn(L"An exception occured while executing '{}' registered by '{}'", aAction, pluginName);
                spdlog::warn(e.what());
            }
            catch (...)
            {
                spdlog::warn(L"An unknown exception occured while executing '{}' registered by '{}'", aAction,
                             pluginName);
            }
        }

        return finished;
    };

    static const std::array<std::function<void(RED4ext::IGameState&, RED4ext::CGameApplication&)>,
                            static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(
                                RED4ext::EGameStateStatus::NextState)>
        stateStatusActions = {[this](RED4ext::IGameState& aState, RED4ext::CGameApplication& aApp)
                              {
                                  if (aState.OnEnter(aApp))
                                  {
                                      m_nextStateStatus = RED4ext::EGameStateStatus::Tick;
                                  }
                                  else
                                  {
                                      m_nextStateStatus = RED4ext::EGameStateStatus::Exit;
                                  }

                                  // Always run original OnEnter only once regardless of the result to emulate game.
                                  m_originalStateActionFinished = true;
                              },
                              [this](RED4ext::IGameState& aState, RED4ext::CGameApplication& aApp)
                              {
                                  m_originalStateActionFinished = aState.OnTick(aApp);

                                  if (m_originalStateActionFinished)
                                      m_nextStateStatus = RED4ext::EGameStateStatus::Exit;
                              },
                              [this](RED4ext::IGameState& aState, RED4ext::CGameApplication& aApp)
                              {
                                  std::ignore = aState.OnExit(aApp);

                                  // Always run original OnExit only once regardless of the result to emulate game.
                                  m_originalStateActionFinished = true;

                                  m_nextStateStatus = RED4ext::EGameStateStatus::NextState;
                              }};

    auto& state = *aApp.currentState;
    auto& stateHooks = GetStateHooks(state.GetType());
    auto& actionHooks = stateHooks.GetActionHooksForStatus(aApp.stateStatus);

    const auto hooksBeforeFinished =
        runHooks(fmt::format(L"{}::OnBefore{}", stateHooks.name, actionHooks.name), actionHooks.onBefore, aApp);

    if (!m_originalStateActionFinished)
    {
        stateStatusActions[static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aApp.stateStatus)](state,
                                                                                                             aApp);
    }

    const auto hooksAfterFinished =
        runHooks(fmt::format(L"{}::OnAfter{}", stateHooks.name, actionHooks.name), actionHooks.onAfter, aApp);

    // Transition state when current action finished execution.
    if (m_originalStateActionFinished && hooksBeforeFinished && hooksAfterFinished)
    {
        // Check if switch of state was enforced by original state action.
        if (m_shouldSwitchState)
        {
            // Override state status.
            aApp.stateStatus = RED4ext::EGameStateStatus::NextState;
            m_shouldSwitchState = false;
        }
        else
        {
            // Normal transition.
            aApp.stateStatus = m_nextStateStatus;
        }

        m_originalStateActionFinished = false;
    }

    // Override successful.
    return true;
}

bool StateSystem::AddHook(std::shared_ptr<PluginBase> aPlugin, const RED4ext::EGameStateType aStateType,
                          const RED4ext::GameState& aState)
{
    auto& stateHooks = GetStateHooks(aStateType);

    if (aState.OnBeforeEnter)
    {
        stateHooks.onEnter.onBefore.emplace_back(aPlugin, aState.OnBeforeEnter);
    }

    if (aState.OnAfterEnter)
    {
        stateHooks.onEnter.onAfter.emplace_back(aPlugin, aState.OnAfterEnter);
    }

    if (aState.OnBeforeTick)
    {
        stateHooks.onTick.onBefore.emplace_back(aPlugin, aState.OnBeforeTick);
    }

    if (aState.OnAfterTick)
    {
        stateHooks.onTick.onAfter.emplace_back(aPlugin, aState.OnAfterTick);
    }

    if (aState.OnBeforeExit)
    {
        stateHooks.onExit.onBefore.emplace_back(aPlugin, aState.OnBeforeExit);
    }

    if (aState.OnAfterExit)
    {
        stateHooks.onExit.onAfter.emplace_back(aPlugin, aState.OnAfterExit);
    }

    spdlog::trace(L"The request to add a '{}' state hooks for plugin '{}' has been successfully completed",
                  stateHooks.name, aPlugin ? aPlugin->GetName() : L"RED4ext");
    return true;
}

void StateSystem::StateHooks::Action::Shutdown()
{
    onBefore.clear();
    onAfter.clear();
}

void StateSystem::StateHooks::Shutdown()
{
    onEnter.Shutdown();
    onTick.Shutdown();
    onExit.Shutdown();
}

#pragma warning(push)
#pragma warning(disable : 4715) // "Not all control paths return a value." -> intentional, should never happen.

StateSystem::StateHooks::Action& StateSystem::StateHooks::GetActionHooksForStatus(
    const RED4ext::EGameStateStatus aStateStatus)
{
    assert(aStateStatus >= RED4ext::EGameStateStatus::Enter && aStateStatus <= RED4ext::EGameStateStatus::Exit &&
           "State status unexpectedly out of range!");

    switch (aStateStatus)
    {
    case RED4ext::EGameStateStatus::Enter:
        return onEnter;
    case RED4ext::EGameStateStatus::Tick:
        return onTick;
    case RED4ext::EGameStateStatus::Exit:
        return onExit;
    }
}

StateSystem::StateHooks& StateSystem::GetStateHooks(const RED4ext::EGameStateType aStateType)
{
    assert(aStateType >= RED4ext::EGameStateType::BaseInitialization &&
           aStateType <= RED4ext::EGameStateType::Shutdown && "State type unexpectedly out of range!");

    switch (aStateType)
    {
    case RED4ext::EGameStateType::BaseInitialization:
        return m_baseInitialization;
    case RED4ext::EGameStateType::Initialization:
        return m_initialization;
    case RED4ext::EGameStateType::Running:
        return m_running;
    case RED4ext::EGameStateType::Shutdown:
        return m_shutdown;
    }
}

#pragma warning(pop)
