#include "CGameApplication.hpp"

#include "App.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"

namespace
{
bool isAttached = false;

bool _CGameApplication_AddState(RED4ext::CGameApplication* aThis, RED4ext::IGameState* aState);
Hook<decltype(&_CGameApplication_AddState)> CGameApplication_AddState(Hashes::CGameApplication_AddState,
                                                                      &_CGameApplication_AddState);
void _CGameApplication_ChangeState(RED4ext::CGameApplication* aThis, RED4ext::EGameStateType aNewState);
Hook<decltype(&_CGameApplication_ChangeState)> CGameApplication_ChangeState(Hashes::CGameApplication_ChangeState,
                                                                            &_CGameApplication_ChangeState);
void _CGameApplication_DoState(RED4ext::CGameApplication* aThis);
Hook<decltype(&_CGameApplication_DoState)> CGameApplication_DoState(Hashes::CGameApplication_DoState,
                                                                    &_CGameApplication_DoState);

bool _CGameApplication_AddState(RED4ext::CGameApplication* aThis, RED4ext::IGameState* aState)
{
    if (aThis && aState)
    {
        switch (aState->GetType())
        {
        case RED4ext::EGameStateType::BaseInitialization:
            [[fallthrough]];
        case RED4ext::EGameStateType::Initialization:
            [[fallthrough]];
        case RED4ext::EGameStateType::Running:
            [[fallthrough]];
        case RED4ext::EGameStateType::Shutdown:
            break;
        default:
        {
            assert(false && "Unknown state encountered!");

            spdlog::warn("Game tries to add state '{}' ({}) which is unknown!", aState->GetName(),
                         static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aState->GetType()));
        }
        }
    }
    else
    {
        assert(aThis && "CGameApplicaiton was nullptr!");
        assert(aState && "IGameState was nullptr!");

        spdlog::error("Unexpected game state addition arguments! Arguments were: aThis = {}, aState = {}.",
                      fmt::ptr(aThis), fmt::ptr(aState));
    }

    return CGameApplication_AddState(aThis, aState);
}

void _CGameApplication_ChangeState(RED4ext::CGameApplication* aThis, const RED4ext::EGameStateType aNextState)
{
    if (!aThis || aNextState < RED4ext::EGameStateType::BaseInitialization ||
        aNextState > RED4ext::EGameStateType::Shutdown)
    {
        if (!aThis)
        {
            assert(false && "CGameApplicaiton was nullptr!");

            spdlog::error("Unexpected game state change arguments! Arguments were: aThis = {}, aNextState = {}.",
                          fmt::ptr(aThis), static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aNextState));
        }

        spdlog::trace("Executing default game state change...");
        CGameApplication_ChangeState(aThis, aNextState);

        if (aThis)
        {
            assert(false && "Unhandled state encountered!");

            spdlog::warn(
                "Executed default game state change as the state '{}' ({}) is unknown!", aThis->currentState->GetName(),
                static_cast<std::underlying_type_t<RED4ext::EGameStateStatus>>(aThis->currentState->GetType()));
        }

        return;
    }

    // Try to override the function.
    if (App::Get()->GetStateSystem()->OnChangeState(*aThis, aNextState))
    {
        // Override successful!
        return;
    }

    // Override failed, execute original.
    spdlog::trace("Executing default game state change...");
    CGameApplication_ChangeState(aThis, aNextState);
}

void _CGameApplication_DoState(RED4ext::CGameApplication* aThis)
{
    if (aThis)
    {
        // Try to override the function.
        if (App::Get()->GetStateSystem()->OnDoState(*aThis))
        {
            // Override successful!
            return;
        }
    }
    else
    {
        assert(false && "CGameApplicaiton was nullptr!");

        spdlog::error("Unexpected game do state arguments! Arguments were: aThis = {}.", fmt::ptr(aThis));
    }

    // Override failed, execute original.
    spdlog::trace("Executing default game do state...");
    CGameApplication_DoState(aThis);
}

bool AttachHook(auto& aHook, const std::string_view aHookName)
{
    spdlog::trace("Trying to attach the hook for the {} at {:#x}...", aHookName, aHook.GetAddress());

    if (const auto result = aHook.Attach(); result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for the {}. Detour error code: {}", aHookName, result);
        return false;
    }

    spdlog::trace("The hook for the {} was attached", aHookName);
    return true;
}

bool DetachHook(auto& aHook, const std::string_view aHookName)
{
    spdlog::trace("Trying to detach the hook for the {} at {:#x}...", aHookName, aHook.GetAddress());

    if (const auto result = aHook.Detach(); result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for the {}. Detour error code: {}", aHookName, result);
        return false;
    }

    spdlog::trace("The hook for the {} was detached", aHookName);
    return true;
}

} // namespace

bool Hooks::CGameApplication::Attach()
{
    if (isAttached)
    {
        return false;
    }

    if (!AttachHook(CGameApplication_AddState, "game states addition"))
    {
        return false;
    }

    if (!AttachHook(CGameApplication_ChangeState, "game state changes"))
    {
        std::ignore = DetachHook(CGameApplication_AddState, "game states addition");
        return false;
    }

    if (!AttachHook(CGameApplication_DoState, "game do state"))
    {
        std::ignore = DetachHook(CGameApplication_ChangeState, "game state changes");
        std::ignore = DetachHook(CGameApplication_AddState, "game states addition");
        return false;
    }

    isAttached = true;
    return true;
}

bool Hooks::CGameApplication::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    if (!DetachHook(CGameApplication_DoState, "game do state"))
    {
        return false;
    }

    if (!DetachHook(CGameApplication_ChangeState, "game state changes"))
    {
        return false;
    }

    if (!DetachHook(CGameApplication_AddState, "game states addition"))
    {
        return false;
    }

    isAttached = false;
    return true;
}
