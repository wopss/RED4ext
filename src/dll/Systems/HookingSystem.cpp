#include "HookingSystem.hpp"

#include "DetourTransaction.hpp"

ESystemType HookingSystem::GetType()
{
    return ESystemType::Hooking;
}

void HookingSystem::Startup()
{
}

void HookingSystem::Shutdown()
{
    std::scoped_lock _(m_mutex);

    spdlog::trace("Detaching {} dangling hook(s)...", m_hooks.size());

    DetourTransaction transaction;
    size_t count = 0;

    for (auto it = m_hooks.begin(); it != m_hooks.end(); ++it)
    {
        auto plugin = it->first;
        auto& item = it->second;

        if (QueueForDetach(plugin, item))
        {
            count++;
        }
    }

    if (transaction.Commit())
    {
        spdlog::trace("{} dangling hook(s) detached", count);
    }
    else
    {
        spdlog::trace("Could not detach {} dangling hook(s)", count);
    }

    m_hooks.clear();
}

bool HookingSystem::Attach(std::shared_ptr<PluginBase> aPlugin, void* aTarget, void* aDetour, void** aOriginal)
{
    spdlog::trace(L"Attaching a hook for '{}' at {} with detour at {}...", aPlugin->GetName(), aTarget, aDetour);
    std::scoped_lock _(m_mutex);

    DetourTransaction transaction;
    Item item(aTarget, aDetour, aOriginal);

    auto result = item.hook.Attach();
    if (result != NO_ERROR)
    {
        spdlog::warn(L"The hook requested by '{}' at {} could not be attached. Detour error code: {}",
                     aPlugin->GetName(), aTarget, result);
        return false;
    }

    if (transaction.Commit())
    {
        if (aOriginal)
        {
            *aOriginal = reinterpret_cast<void*>(item.hook.GetAddress());
        }

        m_hooks.emplace(aPlugin, std::move(item));

        spdlog::trace(L"The hook requested by '{}' at {} has been successfully attached", aPlugin->GetName(), aTarget);
        return true;
    }

    spdlog::warn(L"The hook requested by '{}' at {} was not attached", aPlugin->GetName(), aTarget);
    return false;
}

bool HookingSystem::Detach(std::shared_ptr<PluginBase> aPlugin, void* aTarget)
{
    spdlog::trace(L"Detaching all hooks attached by '{}' at {}...", aPlugin->GetName(), aTarget);
    std::scoped_lock _(m_mutex);

    DetourTransaction transaction;
    size_t count = 0;
    bool hasHook = false;

    auto range = m_hooks.equal_range(aPlugin);
    for (auto it = range.first; it != range.second; ++it)
    {
        auto& item = it->second;
        if (item.target == aTarget)
        {
            hasHook = true;

            if (QueueForDetach(aPlugin, item))
            {
                count++;
            }
        }
    }

    if (!hasHook)
    {
        spdlog::warn(L"No hooks attached by '{}' at {} were found", aPlugin->GetName(), aTarget);
    }
    else if (count == 0)
    {
        spdlog::warn(L"No hooks attached by '{}' at {} were queued for detaching", aPlugin->GetName(), aTarget);
    }
    else if (transaction.Commit())
    {
        spdlog::trace(L"{} hook(s) attached by '{}' at {} have been successfully detached", count, aPlugin->GetName(),
                      aTarget);

        for (auto it = range.first; it != range.second;)
        {
            auto& item = it->second;
            if (item.target == aTarget)
            {
                *item.original = nullptr;
                it = m_hooks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    return count > 0;
}

bool HookingSystem::QueueForDetach(std::shared_ptr<PluginBase> aPlugin, Item& aItem)
{
    auto target = aItem.target;

    spdlog::trace(L"Queueing a hook attached by '{}' at {} for detaching...", aPlugin->GetName(), target);

    auto result = aItem.hook.Detach();
    if (result != NO_ERROR)
    {
        spdlog::warn(L"A hook attached by '{}' at {} could not be detached. Detour error code: {}", aPlugin->GetName(),
                     target, result);
        return false;
    }

    spdlog::trace(L"A hook attached by '{}' at {} has been successfully queued for detaching", aPlugin->GetName(),
                  target);
    return true;
}
