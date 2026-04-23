#include "Cheat.hpp"

#include "Backend/Backend.hpp"
#include "Utils/Hooks.hpp"
#include "Utils/Il2CppHelper.hpp"

#include "Game/Data/Indexes.hpp"
#include "Game/Data/Patterns.hpp"
#include "Game/Data/Structs.hpp"

#include "Game/Pointers/Pointers.hpp"

#include "Game/PixelTime.hpp"
#include "Game/Socket.hpp"

namespace Cheat
{
    void Init()
    {
        // Init
        IL2CPP::INIT();
        Logger::log<ConsoleColor::Info>("Initializing hooks...");

        // Patterns
        Patterns::Init();

        // Pointers
        Pointers::Init();

        // Hooks
        PixelTime::Init();
        Socket::Init();

        Logger::log<ConsoleColor::Info>("Hooks initialized!");
        ImGui::InsertNotification({ ImGuiToastType::Info, 12000, "Clan Utils v1 Loaded, press RCTRL to open/close menu" });
    }
}