#include "Data/Lists.hpp"
#include "Socket.hpp"
#include <Obfusheader.hpp>

using namespace Lists;

// Helpers
std::string RandHex(int hexLength = 4)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    std::stringstream ss; for (int i = 0; i < hexLength; i++)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << distrib(gen);
    }
    
    return ss.str();
}

int Random(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

std::string GetLocalDate()
{
    std::time_t ActualTime = std::time(nullptr);
    std::tm* LocalDate = std::localtime(&ActualTime);

    std::ostringstream oss;
    oss << (LocalDate->tm_mon + 1) << '-' << LocalDate->tm_mday << '-' << (LocalDate->tm_year + 1900);

    return oss.str();
}

// Define Command Creation
json CreateCommand(const CommandsID& id, const json& params, const std::vector<int>& u = {0})
{
    return {{OBF("id"), id}, {OBF("ci"), RandHex()}, {OBF("p"), params}, {OBF("h"), json::object()}, {OBF("u"), u}};
}

json CreateSnapshot(const json& commands)
{
    return json::array({OBF("update_progress"), {{OBF("id"), CommandsID::Snapshot}, {OBF("i"), RandHex()}, {OBF("p"), {{OBF("c"), commands}}}}});
}

// Level Experiences
const int BlackMaketExp[7] = {0, 0, 900, 1800, 5400, 6300, 9375};

// Constants
namespace Constants
{
    const json EmptyCMD = CreateCommand(CommandsID::EmptyCommand, json::object());

    const std::unordered_map<int, std::pair<int, int>> ConsumableMapping = {
        {0, {1, 10}}, {1, {2, 10}}, {2, {3, 10}}, // Chests
        {3, {1030, 1520}}, {4, {2030, 1520}}, {5, {3030, 1520}}, {6, {4030, 1520}}, {7, {6030, 1520}}, // Boosters
        {8, {24015, 1450}}, // VIP
        {9, {17018, 1230}}, // War Hero Chest
        {10, {1024, 1380}}, {11, {2024, 1380}}, {12, {3024, 1380}}, {13, {4024, 1380}}, {14, {5024, 1380}}, {15, {6024, 1380}} // Free Upgrades
    };
    const std::map<int, std::pair<int, int>> FortDataMap = {
        {1, {0, 14}}, {2, {1, 124}}, {3, {2, 4159}}, {4, {3, 7869}}, {5, {4, 20239}},
        {6, {5, 40469}}, {7, {6, 80939}}, {8, {7, 170999}}, {9, {8, 341999}}, {10, {9, 342000}}
    };
    const std::map<int, std::pair<int, int>> TankDataMap = {
        {1, {0, 29}}, {2, {1, 499}}, {3, {2, 1349}}, {4, {3, 5649}}, {5, {4, 10199}},
        {6, {5, 18749}}, {7, {6, 38999}}, {8, {7, 141999}}, {9, {8, 219999}}, {10, {9, 220000}}
    };
}

// Commands
namespace Commands
{
    // Misc
    json Reload()
    {
        return json::array({OBF("update_progress"), json::object()});
    }

    json MySlots(const std::vector<int>& slots = {})
    {
        json result = {
            {OBF("player_id"), Variables::PlayerID}, {OBF("slots"), slots}
        };
        return json::array({OBF("get_progress"), std::move(result)});
    }

    json EmptyCommand()
    {
        return json::array({OBF("update_progress"), Constants::EmptyCMD});
    }

    json Custom()
    {
        return json::array({Variables::EventInput, json::parse(Variables::CommandInput)});
    }
    json Custom1()
    {
        return json::array({Variables::EventInput1, json::parse(Variables::CommandInput1)});
    }
    json Custom2()
    {
        return json::array({Variables::EventInput2, json::parse(Variables::CommandInput2)});
    }
    json Custom3()
    {
        return json::array({ Variables::EventInput3, json::parse(Variables::CommandInput3)});
    }

    // Acc Stuff
    json SetLevel(const int& SelectedLevel)
    {
        json commands;

        for (int level = 1; level <= SelectedLevel; ++level)
        {
            commands.push_back(CreateCommand(CommandsID::UpdateLevel, {{"l", level}}, {140, 6}));
        }
        
        commands.insert(commands.end(), {
            CreateCommand(CommandsID::UpdateExperience, {{"e", 0}, {"ec", 4}, {"ad", json::object()}}, {140, 6}),
            CreateCommand(CommandsID::AnalyticsProgress, {{"eid", 1043}, {"params", {{"ip1", 40}, {"ip2", 140}, {"ip3", 2040}, {"sp1", "MultiplayerMatchReward"}, {"jp1", {{"v_ProgressRoad", 11}, {"v_Tutorial", 12}}}}}}, {140, 6}),
        });

        return CreateSnapshot(commands);
    }

    json Currency(const bool& Spend = false)
    {
        json commands;
        commands.emplace_back(CreateCommand(Spend ? CommandsID::SpendCurrency : CommandsID::AddCurrency, {{"c", Lists::CurrencyList[Variables::CurrencyType]}, {"v", Variables::CurrencyAmount}, {"ca", AddCurrencyCause::GameTask}}));

        return CreateSnapshot(commands);
    }

    json AddModule(const int& Type)
    {
        json commands;
        commands.emplace_back(CreateCommand(CommandsID::InventoryAddItemConsumable, {{"t", 1155}, {"i", Type}, {"c", Variables::ModuleAmount}}));

        return CreateSnapshot(commands);
    }

    json AddAllModules(const int& Amount = 2500)
    {
        json commands;

        for (size_t i = 0; i < Lists::ModuleList.size(); ++i)
        {
            commands.emplace_back(CreateCommand(CommandsID::InventoryAddItemConsumable, {{"t", 1155}, {"i", 346009 + i * 1000}, {"c", Amount}, {"sc", 1}}));
        }

        return CreateSnapshot(commands);
    }

    json UpgradeModule(const int& Type)
    {
        int current_level = Variables::ModuleSlot.value(std::to_string(Type), 1);
        int increase_times = 10 - current_level;

        json commands;

        for (short i = 1; i <= (std::min)(Variables::ModuleLevel, increase_times); ++i)
        {
            commands.emplace_back(CreateCommand(CommandsID::SpendCurrency, {{"c", "Coins"}, {"v", 0}, {"ca", SpendCurrencyCause::UpgradeModule}})); // Saitama does this
            commands.emplace_back(CreateCommand(CommandsID::ModuleInfoIncreaseUp, {{"i", Type}}));
        }
        commands.emplace_back(Constants::EmptyCMD); // In case no upgrades needed

        return CreateSnapshot(commands);
    }

    json UpgradeAllModules()
    {
        json commands;

        for (int module_index = 304; module_index <= 345; ++module_index)
        {
            int current_module = module_index * 1000 + 9;
            int current_level = Variables::ModuleSlot.value(std::to_string(current_module), 1);
            int increase_times = 10 - current_level;

            int max_increase = (std::min)(Variables::ModuleLevel, increase_times);

            for (short i = 0; i < max_increase; ++i)
            {
                commands.emplace_back(CreateCommand(CommandsID::SpendCurrency, {{"c", "Coins"}, {"v", 0}, {"ca", SpendCurrencyCause::UpgradeModule}}));
                commands.emplace_back(CreateCommand(CommandsID::ModuleInfoIncreaseUp, {{"i", current_module}}));
            }
        }
        commands.emplace_back(Constants::EmptyCMD); // In case no upgrades needed

        return CreateSnapshot(commands);
    }

    json AddConsumable(const CommandsID& CommandID, const int& Consumable, const int& Type)
    {
        json commands;
        commands.emplace_back(CreateCommand(CommandID, {{"c", Variables::ConsumableAmount}, {OBF("i"), Consumable}, {OBF("t"), Type} }));

        return CreateSnapshot(commands);
    }

    json SetRank(const int& Rank, const int& Amount)
    {
        json commands;
        commands.emplace_back(CreateCommand(CommandsID::UpdateClanLevelAndExperience, {{OBF("cr"), Rank}, {OBF("cre"), Amount}}));

        return CreateSnapshot(commands);
    }

    json CompleteTutorial()
    {
        json commands;
        auto blacklist = Variables::TutorialSlot.empty() ? std::unordered_set<std::string>() : Variables::TutorialSlot.get<std::unordered_set<std::string>>();

        for (short t = 1100; t <= 2800; t += 100)
        {
            std::string tutorial_slot = std::to_string(t);
            if (blacklist.find(tutorial_slot) == blacklist.end())
            {
                commands.emplace_back(CreateCommand(CommandsID::TutorialComplete, {{OBF("t"), tutorial_slot}}));
            }
        }
        commands.emplace_back(Constants::EmptyCMD); // In case slots are full

        return CreateSnapshot(commands);
    }

    json VeteranBadge(const bool& Remove = false)
    {
        json commands;
        commands.emplace_back(CreateCommand(Remove ? CommandsID::InventoryRemoveItemSingle : CommandsID::InventoryAddItemSingle, {{"i", 700015}, {"ca", 115}}));

        return CreateSnapshot(commands);
    }

    // Stats
    json AddWin(const short& Mode)
    {
        json result = {
            {OBF("mode"), Mode}
        };
        return json::array({OBF("update_player"), std::move(result)});
    }

    json AddWinRaid(const short& Difficulty)
    {
        json result = {
            {OBF("mode"), 22}, {OBF("raid_diff"), Difficulty}
        };
        return json::array({OBF("update_player"), std::move(result)});
    }

    json AddWinTournament()
    {
        json result = {
            {OBF("tournament"), 1}
        };
        return json::array({OBF("update_player"), std::move(result)});
    }

    json UpdateMonthlyStats(const int& Kills, const int& Deaths, const int& HeadShots, const int& KillStreak, const int& WinStreak, const bool& Win = true)
    {
        json commands;
        commands.emplace_back(CreateCommand(CommandsID::UpdatePlayerStatsV2, {{"k", Kills}, {"d", Deaths}, {"w", Win}, {"hds", HeadShots}, {"h", HeadShots}, {"s", KillStreak}, {"ks", KillStreak}, {"c", false}, {"tw", WinStreak}, {"sw", WinStreak}, {"sid", RandHex(16)}, {"dk", GetLocalDate()}}));

        return CreateSnapshot(commands);
    }

    // Clan
    json SetClanFortLevel(const int& Level)
    {
        auto it = Constants::FortDataMap.find(Level);
        if (it != Constants::FortDataMap.end())
        {
            const auto& data = it->second;
            json result = {{OBF("field_level"), data.first}, {OBF("field_points"), data.second}};
            return json::array({OBF("set_clan_building_levels"), result});
        }

        return json::array();
    }

    json SetClanTankLevel(const int& Level)
    {
        auto it = Constants::TankDataMap.find(Level);
        if (it != Constants::TankDataMap.end())
        {
            const auto& data = it->second;
            json result = {{OBF("tank_level"), data.first}, {OBF("tank_points"), data.second} };
            return json::array({OBF("set_clan_building_levels"), result});
        }

        return json::array();
    }

    json EditClan(const std::string& Logo)
    {
        json result = {
            {"logo", Logo}
        };
        return json::array({OBF("change_clan_params"), std::move(result)});
    }

    json EditClanName(const std::string& Name)
    {
        json result = {
            {"name", Name}
        };
        return json::array({OBF("change_clan_params"), std::move(result)});
    }

    json AddClanValor()
    {
        json result = {
            {OBF("clan_id"), Variables::ClanID}, {OBF("war_id"), Variables::ClanWarID}, {OBF("shields"), Variables::ValorAmount}, {OBF("task_id"), Random(1, 10000)}
        };
        return json::array({OBF("warV2_obtain_clan_shields"), std::move(result)});
    }

    json CompleteClanTasks()
    {
        json commands;
        json result = {{OBF("id"), CommandsID::GameTaskChangeInfoInBattle}, {OBF("i"), RandHex()}};

        for (int i = OBF(1); i <= OBF(13); ++i)
        {
            for (int j = OBF(1000); j <= OBF(1200); ++j)
            {
                std::string key = std::to_string(i) + "_" + std::to_string(j);
                commands[key] = {{OBF("p"), OBF(696969)}};
            }
        }

        result[OBF("p")] = commands;
        return json::array({OBF("update_progress"), std::move(result)});
    }
}