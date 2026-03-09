#pragma once

#include <RED4ext/Api/v1/PluginHandle.hpp>

namespace v1
{
namespace Logger
{
void Trace(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void TraceF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void TraceW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void TraceWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Debug(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void DebugF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void DebugW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void DebugWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Info(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void InfoF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void InfoW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void InfoWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Warn(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void WarnF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void WarnW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void WarnWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Error(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void ErrorF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void ErrorW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void ErrorWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Critical(RED4ext::v1::PluginHandle aHandle, const char* aMessage);
void CriticalF(RED4ext::v1::PluginHandle aHandle, const char* aFormat, ...);

void CriticalW(RED4ext::v1::PluginHandle aHandle, const wchar_t* aMessage);
void CriticalWF(RED4ext::v1::PluginHandle aHandle, const wchar_t* aFormat, ...);
} // namespace Logger
} // namespace v1
