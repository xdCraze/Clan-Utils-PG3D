#pragma once
#include <Json.hpp>

namespace Variables
{
	/* JSON */
	inline bool CheckSlots[3] = { false };
	inline nlohmann::ordered_json TutorialSlot;
	inline nlohmann::ordered_json ModuleSlot;

	/* TEXT */
	inline char ClanNameColor[100] = "sigma";
	inline char EventInput[100] = "update_progress";
	inline char EventInput1[100] = "update_progress";
	inline char EventInput2[100] = "create_gifts";
	inline char EventInput3[100] = "update_progress";
	inline char CommandInput[1000000] = "{}";
	inline char CommandInput1[1000000] = "{}";
	inline char CommandInput2[1000000] = "{}";
	inline char CommandInput3[1000000] = "{}";
	inline std::string GameVersion = "", PlayerID = "", PlayerUsername = "", ClanID = "";

	/* VALUES */
	inline int PlayerLevel = 1, AdderLevel = 65;
	inline int CurrencyType, CurrencyAmount = 0;
	inline int ConsumableType, ConsumableAmount = 0;
	inline int GameMode, WinsAmount = 0;
	inline int Kills, Deaths, HeadShots, KillStreak, WinStreak, MonthlyAmount = 0;
	inline int ClanRank = 11, RankPoints = 120000, ClanWarID = 0, ClanLevel = 2;
	inline int ModuleType, ModuleAmount, ModuleLevel = 9;
	inline int ValorAmount = 50, ValorRepeats = 46;
	inline int LogoType = 0;
	inline float DimAmmount = 0.8f;

	/* BOOLEANS */
	inline bool SetRank, SetLevel, CompleteTutorial = false;
	inline bool AddWin, MonthlyStats, MonthlyWins, MonthlyGames = false;
	inline bool AddBadge, RemoveBadge = false;
	inline bool SetClanFort, SetClanTank = false;
	inline bool EditClanLogo, EditClanName = false;
	inline bool AddCurrency, SpendCurrency, AddConsumable, AddModule, AddAllModules, UpgradeModule, UpgradeAllModules = false;
	inline bool AddValor, CompleteClanTasks = false;
	inline bool SendCommand, SendCommand1, SendCommand2, SendCommand3, AutoReload = true, ReloadSocket = false, LogWebsocket = false;
	inline bool Snowflakes, Background = true;

	inline bool Test = false;
}