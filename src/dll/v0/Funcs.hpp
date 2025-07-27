#pragma once

namespace v0
{
namespace Hooking
{
bool Attach(RED4ext::PluginHandle aHandle, void* aTarget, void* aDetour, void** aOriginal);
bool Detach(RED4ext::PluginHandle aHandle, void* aTarget);
} // namespace Hooking

namespace GameStates
{
bool Add(RED4ext::PluginHandle aHandle, RED4ext::EGameStateType aType, RED4ext::GameState* aState);
} // namespace GameStates

namespace Scripts
{
bool Add(RED4ext::PluginHandle aHandle, const wchar_t* aPath);
bool RegisterNeverRefType(const char* aType);
bool RegisterMixedRefType(const char* aType);
} // namespace Scripts

} // namespace v0
