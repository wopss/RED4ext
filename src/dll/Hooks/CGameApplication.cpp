#include "CGameApplication.hpp"
#include "Addresses.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

#include "States/BaseInitializationState.hpp"
#include "States/InitializationState.hpp"
#include "States/RunningState.hpp"
#include "States/ShutdownState.hpp"

namespace
{
bool isAttached = false;

bool _CGameApplication_AddState(RED4ext::CGameApplication* aThis, RED4ext::IGameState* aState);
Hook<decltype(&_CGameApplication_AddState)> CGameApplication_AddState(Hashes::CGameApplication_AddState,
                                                                      &_CGameApplication_AddState);

bool _CGameApplication_AddState(RED4ext::CGameApplication* aThis, RED4ext::IGameState* aState)
{
    bool success = true;
    try
    {
        switch (aState->GetType())
        {
        case RED4ext::EGameStateType::BaseInitialization:
        {
            success =
                States::BaseInitializationState::Attach(static_cast<RED4ext::CBaseInitializationState*>(aState)) &&
                success;
            break;
        }
        case RED4ext::EGameStateType::Initialization:
        {
            success =
                States::InitializationState::Attach(static_cast<RED4ext::CInitializationState*>(aState)) && success;
            break;
        }
        case RED4ext::EGameStateType::Running:
        {
            success = States::RunningState::Attach(static_cast<RED4ext::CRunningState*>(aState)) && success;
            break;
        }
        case RED4ext::EGameStateType::Shutdown:
        {
            success = States::ShutdownState::Attach(static_cast<RED4ext::CShutdownState*>(aState)) && success;
            break;
        }
        default:
        {
            spdlog::warn("State '{}' ({}) is not handled", aState->GetName(), static_cast<int32_t>(aState->GetType()));
        }
        }

        if (success)
        {
            spdlog::trace("All virtual functions were changed successfully");
        }
        else
        {
            spdlog::warn(
                "One or more game state virtual functions could not be changed, the game will continue running "
                "but unexpected behavior might happen");
        }
    }
    catch (const std::exception& e)
    {
        spdlog::warn("An exception occurred while changing the virtual functions for the game states");
        spdlog::warn(e.what());
    }
    catch (...)
    {
        spdlog::warn("An unknown exception occurred while changing the virtual functions for the game states");
    }

    return CGameApplication_AddState(aThis, aState);
}
} // namespace

bool Hooks::CGameApplication::Attach()
{
    spdlog::trace("Trying to attach the hook for the game application at {:#x}...",
                  CGameApplication_AddState.GetAddress());

    auto result = CGameApplication_AddState.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the game application. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the game application was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::CGameApplication::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for the game application at {:#x}...",
                  CGameApplication_AddState.GetAddress());

    auto result = CGameApplication_AddState.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the game application. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for the game application was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
