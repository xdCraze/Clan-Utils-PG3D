#pragma once

#if !defined(_M_IX86) && !defined(_M_X64) && !defined(__i386__) && !defined(__x86_64__)
#error MinHook supports only x86 and x64 systems.
#endif

#include <windows.h>

// MinHook Error Codes.
typedef enum MH_STATUS {
    MH_UNKNOWN = -1,      // Unknown error. Should not be returned.
    MH_OK = 0,            // Successful.
    MH_ERROR_ALREADY_INITIALIZED,
    MH_ERROR_NOT_INITIALIZED,
    MH_ERROR_ALREADY_CREATED,       // The hook for the specified target function is already created.
    MH_ERROR_NOT_CREATED,           // The hook for the specified target function is not created yet.
    MH_ERROR_ENABLED,               // The hook for the specified target function is already enabled.
    MH_ERROR_DISABLED,              // The hook for the specified target function is not enabled yet, or already disabled.
    MH_ERROR_NOT_EXECUTABLE,        // The specified pointer is invalid. It points to a non-allocated/non-executable region.
    MH_ERROR_UNSUPPORTED_FUNCTION,  // The specified target function cannot be hooked.
    MH_ERROR_MEMORY_ALLOC,          // Failed to allocate memory.
    MH_ERROR_MEMORY_PROTECT,        // Failed to change memory protection.
    MH_ERROR_MODULE_NOT_FOUND,      // The specified module is not loaded.
    MH_ERROR_FUNCTION_NOT_FOUND     // The specified function is not found.
} MH_STATUS;

constexpr LPVOID MH_ALL_HOOKS = nullptr;

#ifdef __cplusplus
extern "C" {
#endif

    // Initialize the MinHook library. You must call this function EXACTLY ONCE at the beginning of your program.
    MH_STATUS WINAPI MH_Initialize(void);

    // Uninitialize the MinHook library. You must call this function EXACTLY ONCE at the end of your program.
    MH_STATUS WINAPI MH_Uninitialize(void);

    // Creates a Hook for the specified target function, in disabled state.
    MH_STATUS WINAPI MH_CreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID* ppOriginal);

    // Creates a Hook for the specified API function, in disabled state.
    MH_STATUS WINAPI MH_CreateHookApi(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal);

    // Creates a Hook for the specified API function, in disabled state (Extended).
    MH_STATUS WINAPI MH_CreateHookApiEx(LPCWSTR pszModule, LPCSTR pszProcName, LPVOID pDetour, LPVOID* ppOriginal, LPVOID* ppTarget);

    // Removes an already created hook.
    MH_STATUS WINAPI MH_RemoveHook(LPVOID pTarget);

    // Enables an already created hook.
    MH_STATUS WINAPI MH_EnableHook(LPVOID pTarget);

    // Disables an already created hook.
    MH_STATUS WINAPI MH_DisableHook(LPVOID pTarget);

    // Queues to enable an already created hook.
    MH_STATUS WINAPI MH_QueueEnableHook(LPVOID pTarget);

    // Queues to disable an already created hook.
    MH_STATUS WINAPI MH_QueueDisableHook(LPVOID pTarget);

    // Applies all queued changes in one go.
    MH_STATUS WINAPI MH_ApplyQueued(void);

    // Translates the MH_STATUS to its name as a string.
    const char* WINAPI MH_StatusToString(MH_STATUS status);

#ifdef __cplusplus
}
#endif