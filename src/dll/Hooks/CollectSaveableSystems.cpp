#include "CollectSaveableSystems.hpp"
#include "Addresses.hpp"
#include "Detail/AddressHashes.hpp"
#include "Hook.hpp"
#include "stdafx.hpp"

namespace
{
bool isAttached = false;

void _CollectSaveableSystems(void* a1, const RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>& aAllSystems);
Hook<decltype(&_CollectSaveableSystems)> GameInstance_CollectSaveableSystems(
    Hashes::GameInstance_CollectSaveableSystems, &_CollectSaveableSystems);

void _CollectSaveableSystems(void* a1, const RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>>& aAllSystems)
{
    static constexpr auto MaxSaveableSystems = 160;
    static constexpr auto PreSaveVFuncIndex = 0x130 / sizeof(uintptr_t);

    static RED4ext::UniversalRelocPtr<uintptr_t> IGameSystemVFT(Hashes::IGameSystem_vtbl);
    static uintptr_t DefaultPreSaveVFunc = IGameSystemVFT.GetAddr()[PreSaveVFuncIndex];

    RED4ext::DynArray<RED4ext::Handle<RED4ext::IScriptable>> saveableSystems;

    for (const auto& system : aAllSystems)
    {
        auto systemVFT = *reinterpret_cast<uintptr_t**>(system.instance);

        if (systemVFT[PreSaveVFuncIndex] == DefaultPreSaveVFunc)
            continue;

        saveableSystems.PushBack(system);

        if (saveableSystems.size == MaxSaveableSystems)
            break;
    }

    GameInstance_CollectSaveableSystems(a1, saveableSystems);
}
} // namespace

bool Hooks::CollectSaveableSystems::Attach()
{
    spdlog::trace("Trying to attach the hook for collect saveable systems at {:#x}...",
                  GameInstance_CollectSaveableSystems.GetAddress());

    auto result = GameInstance_CollectSaveableSystems.Attach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not attach the hook for collect saveable systems. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for collect saveable systems was attached");
    }

    isAttached = result == NO_ERROR;
    return isAttached;
}

bool Hooks::CollectSaveableSystems::Detach()
{
    if (!isAttached)
    {
        return false;
    }

    spdlog::trace("Trying to detach the hook for collect saveable systems at {:#x}...",
                  GameInstance_CollectSaveableSystems.GetAddress());

    auto result = GameInstance_CollectSaveableSystems.Detach();
    if (result != NO_ERROR)
    {
        spdlog::error("Could not detach the hook for collect saveable systems. Detour error code: {}", result);
    }
    else
    {
        spdlog::trace("The hook for collect saveable systems was detached");
    }

    isAttached = result != NO_ERROR;
    return !isAttached;
}
