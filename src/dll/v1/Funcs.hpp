#pragma once

#include <RED4ext/Api/v1/GameState.hpp>
#include <RED4ext/Api/v1/PluginHandle.hpp>
#include <RED4ext/GameStates.hpp>

namespace v1
{
namespace Hooking
{
bool Attach(RED4ext::v1::PluginHandle aHandle, void* aTarget, void* aDetour, void** aOriginal);
bool Detach(RED4ext::v1::PluginHandle aHandle, void* aTarget);
} // namespace Hooking

namespace GameStates
{
bool Add(RED4ext::v1::PluginHandle aHandle, RED4ext::EGameStateType aType, RED4ext::v1::GameState* aState);
} // namespace GameStates

namespace Scripts
{
bool Add(RED4ext::v1::PluginHandle aHandle, const wchar_t* aPath);
bool RegisterNeverRefType(const char* aType);
bool RegisterMixedRefType(const char* aType);
} // namespace Scripts

} // namespace v1
