#pragma once

#include <windows.h>
#include <lmcons.h>
#include <thread>
#include <iostream>
#include <intrin.h>
#include <vector>
#include <list>
#include <random>
#include <commdlg.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <functional>
#include <map>
#include <set>
#include <atomic>
#include <unordered_set>
#include <shellapi.h>
#include <psapi.h>
#include <Json.hpp>
#include <Auth.hpp>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include "../../Dependencies/Libs/curl/curl.h"
#include "../../Dependencies/Libs/discord-rpc/discord_rpc.h"
#include "../../Dependencies/Libs/discord-rpc/discord_register.h"

#include "../Game/Data/Lists.hpp"
#include "../Game/Data/Structs.hpp"
#include "../Game/Data/Variables.hpp"

#include <ImGui/imgui.hpp>
#include <ImGui/imgui_internal.hpp>
#include <ImGui/Notify/imgui_notify.hpp>
#include <ImGui/Implementation/imgui_impl_win32.hpp>
#include <ImGui/Implementation/imgui_impl_dx11.hpp>

class Backend
{
    public:
        bool Load();
        bool DirectXPresentHook();
        void LoadImGui(HWND window, ID3D11Device* device, ID3D11DeviceContext* context);
        void DrawImGui(ID3D11DeviceContext* context, ID3D11RenderTargetView* targetview) const;

        using presentVariable = long(__stdcall*)(IDXGISwapChain*, UINT, UINT);

        // Member variables
        DXGI_SWAP_CHAIN_DESC SwapChainDescription{};
        IDXGISwapChain* SwapChain = nullptr;
        ID3D11Device* Device = nullptr;
        const D3D_FEATURE_LEVEL FeatureLevels[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        HWND Window = nullptr;
        ID3D11DeviceContext* PointerContext = nullptr;
        ID3D11RenderTargetView* MainRenderTargetView = nullptr;
        WNDPROC originalWndProc;
        DXGI_SWAP_CHAIN_DESC PresentHookSwapChain;

        // Open Menu
        bool OpenMenu = false;

        // Constants
        static const HWND MAIN_WINDOW;
        const int WIDTH = GetSystemMetrics(SM_CXSCREEN);
        const int HEIGHT = GetSystemMetrics(SM_CYSCREEN);
        const POINT CENTER = { static_cast<LONG>(WIDTH * 0.5), static_cast<LONG>(HEIGHT * 0.5) };

        // Functions
        void CursorFix(bool clip);

        // Enums for UI management
        const enum Heads { Websocket, Settings };
        const enum WebsocketSubs { NotEnabled, Basic, Stats, Clan, Misc };
        const enum SettingsSubs { Settings1, Settings2, Settings3 };
};

extern Backend Features;