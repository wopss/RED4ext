#include "stdafx.hpp"
#include "ShutdownState.hpp"
#include "App.hpp"
#include "GameStateHook.hpp"

namespace
{
GameStateHook<RED4ext::CShutdownState> CShutdownState(&States::ShutdownState::OnEnter, &States::ShutdownState::OnUpdate,
                                                      &States::ShutdownState::OnExit);
}

bool States::ShutdownState::OnEnter(RED4ext::CShutdownState* aThis, RED4ext::CGameApplication* aApp)
{
    auto app = App::Get();
    auto stateSystem = app->GetStateSystem();

    stateSystem->OnEnter(RED4ext::EGameStateType::Shutdown, aApp);
    return CShutdownState.OnEnter(aThis, aApp);
}

bool States::ShutdownState::OnUpdate(RED4ext::CShutdownState* aThis, RED4ext::CGameApplication* aApp)
{
    auto app = App::Get();
    auto stateSystem = app->GetStateSystem();

    auto result = stateSystem->OnUpdate(RED4ext::EGameStateType::Shutdown, aApp);
    result = result && CShutdownState.OnUpdate(aThis, aApp);

    /*
     * Doing this because the game might call "SetState" which will also change the application status and will force
     * the state to move on.
     */
    if (!result)
    {
        aApp->status = RED4ext::EGameStateStatus::Initialized;
        return false;
    }

    aApp->status = RED4ext::EGameStateStatus::Ran;
    return true;
}

bool States::ShutdownState::OnExit(RED4ext::CShutdownState* aThis, RED4ext::CGameApplication* aApp)
{
    auto app = App::Get();
    auto stateSystem = app->GetStateSystem();

    stateSystem->OnExit(RED4ext::EGameStateType::Shutdown, aApp);
    return CShutdownState.OnExit(aThis, aApp);
}

bool States::ShutdownState::Attach(RED4ext::CShutdownState* aState)
{
    return CShutdownState.AttachAt(aState);
}

bool States::ShutdownState::Detach(RED4ext::CShutdownState* aState)
{
    return CShutdownState.DetachAt(aState);
}
