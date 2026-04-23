#pragma once

#include <MinHook.hpp>
#include <Logger/Logger.hpp>

inline bool CheckPointer(void* pointer, const char* context)
{
#ifdef _DEBUG
    if (pointer == nullptr)
    {
        //Logger::log<ConsoleColor::Error>("Error at %s: pointer is null (%p)", context, pointer);
        return false;
    }
#endif
    return true;
}

inline bool InitializeMinHook()
{
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK)
    {
#ifdef _DEBUG
        //Logger::log<ConsoleColor::Error>("Error initializing MinHook: %d", status);
#endif
        return false;
    }
    return true;
}

inline void LogMinHookResult(MH_STATUS status, const char* context, void* pointer)
{
#ifdef _DEBUG
    if (status != MH_OK)
    {
        //Logger::log<ConsoleColor::Error>("Error %s function at %p: %d", context, pointer, status);
    }
    else
    {
        //Logger::log<ConsoleColor::Info>("Successfully %s function at %p", context, pointer);
    }
#endif
}

template<typename Func>
void Detour(void* pointer, Func* detour)
{
    if (!CheckPointer(pointer, "detouring"))
        return;

    MH_STATUS status = MH_CreateHook(pointer, (LPVOID)detour, nullptr);
    //LogMinHookResult(status, "creating detour", pointer);

    if (status != MH_OK)
        return;

    status = MH_EnableHook(pointer);
   // LogMinHookResult(status, "enabling detour", pointer);
}

template<typename Func>
void AttachHook(void* pointer, Func* detour, Func** original)
{
    if (!CheckPointer(pointer, "hooking"))
        return;

    MH_STATUS status = MH_CreateHook(pointer, (LPVOID)detour, (LPVOID*)original);
    //LogMinHookResult(status, "creating hook", pointer);

    if (status != MH_OK)
        return;

    status = MH_EnableHook(pointer);
    //LogMinHookResult(status, "enabling hook", pointer);
}