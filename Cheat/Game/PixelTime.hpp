#pragma once
#include "Commands.hpp"
#include <Obfusheader.hpp>

namespace PixelTime
{
    // Check My Progress Slots
    void CheckMySlots(const int& slot, bool& function)
    {
        Socket::SendCommand(Commands::MySlots({slot}), [&](Structs::Socket::Response packet)
        {
            if (packet.ResponseStatus == OBF("OK"))
            {
                function = true;
            }
        });
    }

    // Functions
    VOID(*PixelTime_o)(IL2CPP::Object* Instance);
    VOID PixelTime(IL2CPP::Object* Instance)
    {
        if (Instance != nullptr && Pointers::SocketInstance != nullptr)
        {
            // Pointer Values
            Variables::GameVersion = Pointers::Profile::GetVersion()->ToString();
            Variables::PlayerID = Pointers::Profile::GetID()->ToString();
            Variables::PlayerUsername = Pointers::Profile::GetUsername()->ToString();
            Variables::PlayerLevel = Pointers::Profile::GetLevel();

            // Reload
            if (Variables::ReloadSocket)
            {
                Socket::SendCommand(Commands::Reload());
                ImGui::InsertNotification({ ImGuiToastType::Info, 3000, OBF("Reloading socket...") });
                Variables::ReloadSocket = false;
            }

            // Adders
            if (Variables::AddCurrency)
            {
                Socket::SendNotifCommand(OBF("Add Currency"), Commands::Currency());
                Variables::AddCurrency = false;
            }

            if (Variables::SpendCurrency)
            {
                Socket::SendNotifCommand(OBF("Spend Currency"), Commands::Currency(true));
                Variables::SpendCurrency = false;
            }

            if (Variables::AddModule)
            {
                Socket::SendNotifCommand(OBF("Add Module"), Commands::AddModule(346009 + (Variables::ModuleType * 1000)));
                Variables::AddModule = false;
            }

            if (Variables::AddAllModules)
            {
                Socket::SendNotifCommand(OBF("Add All Modules"), Commands::AddAllModules());
                Variables::AddAllModules = false;
            }

            if (Variables::UpgradeModule)
            {
                Socket::SendNotifCommand(OBF("Upgrade Module"), Commands::UpgradeModule(304009 + (Variables::ModuleType * 1000)));
                Variables::UpgradeModule = false;
            }

            if (Variables::UpgradeAllModules)
            {
                Socket::SendNotifCommand(OBF("Upgrade All Modules"), Commands::UpgradeAllModules());
                Variables::UpgradeAllModules = false;
            }

            if (Variables::AddConsumable)
            {
                CommandsID ID = CommandsID::InventoryAddItemConsumable;
                int I = 0, T = 0;

                if (Constants::ConsumableMapping.count(Variables::ConsumableType))
                {
                    std::tie(I, T) = Constants::ConsumableMapping.at(Variables::ConsumableType);

                    if (Variables::ConsumableType < 3)
                    {
                        ID = CommandsID::GameEvents_UpdateFreeSpin;
                    }

                    Socket::SendNotifCommand(OBF("Add Consumable"), Commands::AddConsumable(ID, I, T));
                }

                Variables::AddConsumable = false;
            }

            if (Variables::SetRank)
            {
                Socket::SendNotifCommand(OBF("Set Rank"), Commands::SetRank(Variables::ClanRank, Variables::RankPoints));
                Variables::SetRank = false;
            }

            // Stats
            if (Variables::CompleteTutorial)
            {
                Socket::SendNotifCommand(OBF("Complete Tutorial"), Commands::CompleteTutorial());
                Variables::CompleteTutorial = false;
            }

            if (Variables::AddBadge)
            {
                Socket::SendNotifCommand(OBF("Add Veteran Badge"), Commands::VeteranBadge());
                Variables::AddBadge = false;
            }

            if (Variables::RemoveBadge)
            {
                Socket::SendNotifCommand(OBF("Remove Veteran Badge"), Commands::VeteranBadge(true));
                Variables::RemoveBadge = false;
            }

            if (Variables::SetLevel)
            {
                Socket::SendNotifCommand(OBF("Set Level"), Commands::SetLevel(Variables::AdderLevel));
                Variables::SetLevel = false;
            }

            if (Variables::AddWin)
            {
                const std::array<int, 10> GameModeMapping = { 0, 2, 8, 5, 31, 20, 16, 1, 2, 3 };
                const int maxGameMode = static_cast<int>(GameModeMapping.size());

                int GameModeWin = (Variables::GameMode < maxGameMode) ? GameModeMapping[Variables::GameMode] : 0;

                ImGui::InsertNotification({ ImGuiToastType::Info, 3000, OBF("Adding Wins, please wait...") });
                json Command;
                if (Variables::GameMode < 7) {
                    Command = Commands::AddWin(GameModeWin);
                }
                else if (Variables::GameMode < 10) {
                    Command = Commands::AddWinRaid(GameModeWin);
                }
                else {
                    Command = Commands::AddWinTournament();
                }

                for (int i = 0; i < Variables::WinsAmount; ++i) {
                    Socket::SendCommand(Command);
                }

                Variables::AddWin = false;
            }

            if (Variables::MonthlyStats)
            {
                Socket::SendNotifCommand(OBF("Update Monthly Stats"), Commands::UpdateMonthlyStats(Variables::Kills, Variables::Deaths, Variables::HeadShots, Variables::KillStreak, Variables::WinStreak));
                Variables::MonthlyStats = false;
            }

            // Websocket - Clan
            if (Variables::SetClanFort)
            {
                Socket::SendNotifCommand(OBF("Set Clan Fort Level"), Commands::SetClanFortLevel(Variables::ClanLevel), false);
                Variables::SetClanFort = false;
            }

            if (Variables::SetClanTank)
            {
                Socket::SendNotifCommand(OBF("Set Clan Tank Level"), Commands::SetClanTankLevel(Variables::ClanLevel), false);
                Variables::SetClanTank = false;
            }

            if (Variables::EditClanLogo)
            {
                Socket::SendNotifCommand(OBF("Edit Clan Logo"), Commands::EditClan(Lists::ClanLogos[Variables::LogoType]), false);
                Variables::EditClanLogo = false;
            }

            if (Variables::EditClanName)
            {
                Socket::SendNotifCommand(OBF("Edit Clan Name"), Commands::EditClanName(Variables::ClanNameColor), false);
                Variables::EditClanName = false;
            }

            if (Variables::AddValor)
            {
                ImGui::InsertNotification({ ImGuiToastType::Info, 3000, OBF("Trying to Add Valor...") });

                for (short i = 0; i < Variables::ValorRepeats; ++i)
                {
                    Socket::SendCommand(Commands::AddClanValor());
                }

                Variables::AddValor = false;
            }

            if (Variables::CompleteClanTasks)
            {
                Socket::SendNotifCommand(OBF("Complete Clan Tasks"), Commands::CompleteClanTasks());
                Variables::CompleteClanTasks = false;
            }

            // Slots Fetchers
            if (Variables::CheckSlots[0])
            {
                CheckMySlots(OBF(56), Variables::CompleteTutorial);
                Variables::CheckSlots[0] = false;
            }

            if (Variables::CheckSlots[1])
            {
                CheckMySlots(OBF(37), Variables::UpgradeModule);
                Variables::CheckSlots[1] = false;
            }

            if (Variables::CheckSlots[2])
            {
                CheckMySlots(OBF(37), Variables::UpgradeAllModules);
                Variables::CheckSlots[2] = false;
            }

            // Custom
            if (Variables::SendCommand)
            {
                Socket::SendNotifCommand(OBF("Custom Command"), Commands::Custom());
                Variables::SendCommand = false;
            }
            if (Variables::SendCommand1)
            {
                Socket::SendNotifCommand(OBF("Custom Command1"), Commands::Custom1());
                Variables::SendCommand1 = false;
            }
            if (Variables::SendCommand2)
                {
                    Socket::SendNotifCommand(OBF("Custom Command2"), Commands::Custom2());
                    Variables::SendCommand2 = false;
            }
            if (Variables::SendCommand3)
                {
                    Socket::SendNotifCommand(OBF("Custom Command3"), Commands::Custom3());
                    Variables::SendCommand3 = false;
                }
        }

        return PixelTime_o(Instance);
    }

    // Initializate
    void Init()
    {
        HookMethod("PixelTime", "Update", PixelTime, (LPVOID*)&PixelTime_o);
    }
}