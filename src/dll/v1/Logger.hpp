#pragma once

namespace v1
{
namespace Logger
{
void Trace(RED4ext::PluginHandle aHandle, const char* aMessage);
void TraceF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void TraceW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void TraceWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Debug(RED4ext::PluginHandle aHandle, const char* aMessage);
void DebugF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void DebugW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void DebugWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Info(RED4ext::PluginHandle aHandle, const char* aMessage);
void InfoF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void InfoW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void InfoWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Warn(RED4ext::PluginHandle aHandle, const char* aMessage);
void WarnF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void WarnW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void WarnWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Error(RED4ext::PluginHandle aHandle, const char* aMessage);
void ErrorF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void ErrorW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void ErrorWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);

void Critical(RED4ext::PluginHandle aHandle, const char* aMessage);
void CriticalF(RED4ext::PluginHandle aHandle, const char* aFormat, ...);

void CriticalW(RED4ext::PluginHandle aHandle, const wchar_t* aMessage);
void CriticalWF(RED4ext::PluginHandle aHandle, const wchar_t* aFormat, ...);
} // namespace Logger
} // namespace v1
