#pragma once

#include "Hook.hpp"
#include "ISystem.hpp"
#include "PluginBase.hpp"

class HookingSystem final : public ISystem
{
public:
    ESystemType GetType() override;

    void Startup() override;
    void Shutdown() override;

    bool Attach(std::shared_ptr<PluginBase> aPlugin, void* aTarget, void* aDetour, void** aOriginal);
    bool Detach(std::shared_ptr<PluginBase> aPlugin, void* aTarget);

private:
    struct Item
    {
        Item(void* aTarget, void* aDetour, void** aOriginal)
            : target(aTarget)
            , original(aOriginal)
            , hook(aTarget, aDetour)
        {
        }

        void* target;
        void** original;
        Hook<void*> hook;
    };

    using Map_t = std::unordered_multimap<std::shared_ptr<PluginBase>, Item>;
    using MapIter_t = Map_t::iterator;

    bool QueueForDetach(std::shared_ptr<PluginBase> aPlugin, Item& aItem);

    std::mutex m_mutex;
    Map_t m_hooks;
};
