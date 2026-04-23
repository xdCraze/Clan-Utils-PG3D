#pragma once

#include "Font/Byte.hpp"
#include "Font/FontAwesome.hpp"
#include "Snowflake.hpp"
#include <Obfusheader.hpp>
#include <curl/curl.h>
#include <fstream>
#include <string>
#include <Windows.h>
#include <Iphlpapi.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include <random>


#pragma comment(lib, "iphlpapi.lib")

using json = nlohmann::json;

// Variables
static float MenuAlpha = 0.0f;
static float DimAlpha = 0.0f;
static bool IsLoggedIn = false;
static char UsernameBuffer[128] = "";
static char PasswordBuffer[128] = "";
static std::string UserExpiry = "";
static std::string UserName = "";
static bool SaveCredentials = true;
static bool IsLoggingIn = false;
static bool LoginError = false;
static std::string LoginErrorMessage = "";

// Auth variables
struct AuthData {
    std::string username;
    std::string password;
    std::string hwid;
};

static AuthData CurrentAuth;

// Functions
template <typename T>
T Clamp(const T& value, const T& min, const T& max)
{
    return value < min ? min : (value > max ? max : value);
}

// CURL callback for HTTP responses
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Get Hardware ID (mix of MAC address and volume serial)
// Helper function to get current timestamp string
std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Logging function
void LogToFile(const std::string& message) {
    std::ofstream logFile("ClanUtilsDebugLog.txt", std::ios::app); // Open in append mode
    if (logFile.is_open()) {
        logFile << "[" << GetTimestamp() << "] " << message << std::endl;
        logFile.close();
    }
    else {
        // Fallback or error handling if log file cannot be opened
        std::cerr << "Error opening log file!" << std::endl;
    }
}

// Get Hardware ID (mix of MAC address and volume serial)
std::string GetHardwareID() {
    //LogToFile("GetHardwareID: Function started.");
    // Get MAC Address
    IP_ADAPTER_INFO adapterInfo[16];
    DWORD dwBufLen = sizeof(adapterInfo);

    std::string hwid = "";

    if (GetAdaptersInfo(adapterInfo, &dwBufLen) == NO_ERROR) {
        ////LogToFile("GetHardwareID: GetAdaptersInfo successful.");
        PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
        while (pAdapterInfo) {
            char macStr[18];
            sprintf_s(macStr, "%02X-%02X-%02X-%02X-%02X-%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                pAdapterInfo->Address[4], pAdapterInfo->Address[5]);

            hwid += macStr;
           // //LogToFile("GetHardwareID: Found MAC Address: " + std::string(macStr));
            break; // Only need one MAC address

            pAdapterInfo = pAdapterInfo->Next;
        }
    }
    else {
        ////LogToFile("GetHardwareID: GetAdaptersInfo failed. Error code: " + std::to_string(GetLastError()));
    }

    // Get Volume Info
    char volumeName[MAX_PATH + 1] = { 0 };
    char fileSystemName[MAX_PATH + 1] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLen = 0;
    DWORD fileSystemFlags = 0;

    if (GetVolumeInformationA("C:\\", volumeName, ARRAYSIZE(volumeName), &serialNumber,
        &maxComponentLen, &fileSystemFlags, fileSystemName, ARRAYSIZE(fileSystemName))) {
        //LogToFile("GetHardwareID: GetVolumeInformationA successful.");
        char serialStr[9];
        sprintf_s(serialStr, "%08X", serialNumber);
        hwid += "-";
        hwid += serialStr;
        //LogToFile("GetHardwareID: Found Volume Serial: " + std::string(serialStr));
    }
    else {
        //LogToFile("GetHardwareID: GetVolumeInformationA failed. Error code: " + std::to_string(GetLastError()));
    }

    //LogToFile("GetHardwareID: Final HWID: " + hwid);
    //LogToFile("GetHardwareID: Function finished.");
    return hwid;
}

// Save login credentials to file
void SaveLoginCredentials(const std::string& username, const std::string& password) {
    //LogToFile("SaveLoginCredentials: Function started. Saving Username: " + username); // Avoid logging password directly if possible
    std::ofstream file("ClanUtilsLogin.txt");
    if (file.is_open()) {
        file << username << std::endl;
        file << password << std::endl;
        file.close();
        //LogToFile("SaveLoginCredentials: Credentials saved successfully to ClanUtilsLogin.txt");
    }
    else {
        //LogToFile("SaveLoginCredentials: Failed to open ClanUtilsLogin.txt for writing.");
    }
    //LogToFile("SaveLoginCredentials: Function finished.");
}

// Load login credentials from file
bool LoadLoginCredentials(std::string& username, std::string& password) {
    //LogToFile("LoadLoginCredentials: Function started.");
    std::ifstream file("ClanUtilsLogin.txt");
    if (file.is_open()) {
        std::getline(file, username);
        std::getline(file, password);
        file.close();
        bool success = !username.empty() && !password.empty();
        if (success) {
            //LogToFile("LoadLoginCredentials: Successfully loaded Username: " + username); // Avoid logging password
        }
        else {
            //LogToFile("LoadLoginCredentials: Loaded credentials but username or password was empty.");
        }
        //LogToFile("LoadLoginCredentials: Function finished. Result: " + std::string(success ? "true" : "false"));
        return success;
    }
    //LogToFile("LoadLoginCredentials: Failed to open ClanUtilsLogin.txt for reading.");
    //LogToFile("LoadLoginCredentials: Function finished. Result: false");
    return false;
}

// Authenticate with server
bool AuthenticateUser(const std::string& username, const std::string& password, const std::string& deviceId, std::string& errorMsg) {
    //LogToFile("AuthenticateUser: Function started. Username: " + username + ", HWID: " + deviceId); // Avoid logging password
    CURL* curl = curl_easy_init();
    if (!curl) {
        errorMsg = "Failed to initialize CURL";
        //LogToFile("AuthenticateUser: Failed to initialize CURL.");
        //LogToFile("AuthenticateUser: Function finished. Result: false");
        return false;
    }

    std::string readBuffer;
    // WARNING: Logging the raw password here for debugging. Remove/mask this in production.
    std::string postFields = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\",\"deviceid\":\"" + deviceId + "\"}";
    //LogToFile("AuthenticateUser: Sending POST data: " + postFields);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: myModMenu");

    curl_easy_setopt(curl, CURLOPT_URL, "https://voidpg.online/api/login");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); 
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); 

    //LogToFile("AuthenticateUser: Performing CURL request to https://voidpg.online/api/login");
    CURLcode res = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);

    curl_slist_free_all(headers); // Free headers after use
    // curl_easy_cleanup(curl); // Cleanup moved lower after potential notifyCurl

    //LogToFile("AuthenticateUser: CURL request completed. CURLCode: " + std::to_string(res) + " (" + curl_easy_strerror(res) + "), HTTP Code: " + std::to_string(httpCode));
    //LogToFile("AuthenticateUser: Raw Response Data: " + readBuffer);

    if (res != CURLE_OK) {
        errorMsg = "Connection error: " + std::string(curl_easy_strerror(res));
        //LogToFile("AuthenticateUser: CURL error occurred: " + errorMsg);
        curl_easy_cleanup(curl); // Cleanup curl handle here on error
        //LogToFile("AuthenticateUser: Function finished. Result: false");
        return false;
    }

    try {
        //LogToFile("AuthenticateUser: Attempting to parse JSON response.");
        json response = json::parse(readBuffer); // This is where the "Failed to parse response" likely originates if readBuffer is not valid JSON
        //LogToFile("AuthenticateUser: JSON parsing successful.");

        
    }
    catch (const json::parse_error& e) { // Catch specific JSON parse error
        errorMsg = "Failed to parse response: " + std::string(e.what());
        //LogToFile("AuthenticateUser: JSON Parsing Exception caught: " + std::string(e.what()) + ". Raw response was: " + readBuffer);
        curl_easy_cleanup(curl); // Cleanup main curl handle
        //LogToFile("AuthenticateUser: Function finished. Result: false");
        return false;
    }
    catch (const std::exception& e) {
        errorMsg = "Generic exception during response processing: " + std::string(e.what());
        //LogToFile("AuthenticateUser: Generic Exception caught: " + std::string(e.what()) + ". Raw response was: " + readBuffer);
        curl_easy_cleanup(curl); // Cleanup main curl handle
        //LogToFile("AuthenticateUser: Function finished. Result: false");
        return false;
    }
}


// Constants
constexpr int CHILD_X = 170;
constexpr float CHILD_SPACED_X = 458.5;
constexpr int FULL_CHILD_BASE_X = 567;
constexpr float HALF_CHILD_BASE_X = 278.5;
constexpr int FULL_BUTTON_LINE = 547;
constexpr float HALF_BUTTON_LINE = 258.5;
constexpr float QUART_BUTTON_LINE = 125.25;

// Draw Menu
void Backend::DrawImGui(ID3D11DeviceContext* context, ID3D11RenderTargetView* targetview) const
{
    if (!context || !targetview) return;

    // ImGui setup
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
   
    // Static state variables
    static Heads Tab = Websocket;
    static float TabAnim = -10.0f;
    static int PreviousTab = -1;
    static bool ConsoleInitialized = false;
    static bool DelayComplete = false;
    static bool InitialAuthCheck = false;
    static std::string hwid = GetHardwareID();

    if (!ConsoleInitialized && !DelayComplete)
    {
        Sleep(1000);
        DelayComplete = true;
    }

    if (!ConsoleInitialized && DelayComplete)
    {
        AllocConsole(); // Allocate a console window
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);
        std::cout << "Menu has opened Successfully!\n";
        ConsoleInitialized = true;

        // Load saved credentials
        std::string savedUsername, savedPassword;
        if (LoadLoginCredentials(savedUsername, savedPassword)) {
            strncpy_s(UsernameBuffer, savedUsername.c_str(), sizeof(UsernameBuffer));
            strncpy_s(PasswordBuffer, savedPassword.c_str(), sizeof(PasswordBuffer));
        }
    }

    const float deltaTime = ImGui::GetIO().DeltaTime;
    const float openSpeed = 14.0f * deltaTime;
    const float closeSpeed = 9999.0f * deltaTime;

    // Attempt to login with saved credentials if available
    if (!InitialAuthCheck && UsernameBuffer[0] != '\0' && PasswordBuffer[0] != '\0') {
        InitialAuthCheck = true;
        std::string errorMsg;
        IsLoggingIn = true;

        if (AuthenticateUser(UsernameBuffer, PasswordBuffer, hwid, errorMsg)) {
            IsLoggedIn = true;
            UserName = UsernameBuffer;
            UserExpiry = "Active";  // We'll need to get actual expiry from API
            IsLoggingIn = false;
        }
        else {
            LoginError = true;
            LoginErrorMessage = errorMsg;
            IsLoggingIn = false;
        }
    }

    // Handle background dim and menu alpha
    if (Features.OpenMenu)
    {
        DimAlpha = Clamp(DimAlpha + openSpeed, 0.0f, Variables::DimAmmount);
        MenuAlpha = Clamp(MenuAlpha + openSpeed, 0.0f, 1.0f);
    }
    else
    {
        DimAlpha = Clamp(DimAlpha - closeSpeed, 0.0f, Variables::DimAmmount);
        MenuAlpha = Clamp(MenuAlpha - closeSpeed, 0.0f, 1.0f);
    }

    // Background Effects (Drawn even if menu is closed/closing for dimming)
    if (DimAlpha > 0.0f) {
        if (Variables::Background) ImGui::GetBackgroundDrawList()->AddRectFilled({ 0, 0 }, { ImVec2(Features.WIDTH, Features.HEIGHT) }, ImColor(0.0f, 0.0f, 0.0f, DimAlpha));
    }
    if (MenuAlpha > 0.0f && Variables::Snowflakes) { // Only draw snowflakes if menu is visible
        Snowflake::Snowflakes();
    }


    if (MenuAlpha > 0.0f)
    {
        if (!IsLoggedIn) {
            // Login window
            ImGui::SetNextWindowSize(ImVec2(320, 200));
            ImGui::SetNextWindowBgAlpha(MenuAlpha); // Use MenuAlpha for window background
            ImGui::Begin("ClanUtils Login", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            {
                auto draw = ImGui::GetWindowDrawList();
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();

                // Shadow/Glow Effect logic specific to login window
                ImVec4 brightRedVec = ImVec4(0.1f, 0.6f, 1.0f, MenuAlpha);
                ImVec4 deepRedVec = ImVec4(0.1f, 0.3f, 0.8f, MenuAlpha);

                static float time = 0.0f;
                time += deltaTime * 3.0f;

                float t = (sin(time) * 0.5f) + 0.5f;
                t = t * t * (3 - 2 * t);

                ImVec4 flickerColorVec = ImVec4(brightRedVec.x + (deepRedVec.x - brightRedVec.x) * t, brightRedVec.y + (deepRedVec.y - brightRedVec.y) * t, brightRedVec.z + (deepRedVec.z - brightRedVec.z) * t, MenuAlpha);
                ImColor flickerColor = ImColor(flickerColorVec.x, flickerColorVec.y, flickerColorVec.z, flickerColorVec.w);

                const float newWidth = 320 * 1.02f;
                const float newHeight = 200 * 1.02f;
                const float offsetX = (newWidth - 320) * 0.5f;
                const float offsetY = (newHeight - 200) * 0.5f;
                const float rounding = 10.0f;
                const ImDrawFlags flags = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight;

                // Draw shadow
                ImGui::GetBackgroundDrawList()->AddShadowRect(ImVec2(pos.x - offsetX, pos.y - offsetY), ImVec2(pos.x + 320 + offsetX, pos.y + 200 + offsetY), flickerColor, 100, ImVec2(0, 0), flags, rounding);

                // Title
                ImGui::SetCursorPos(ImVec2(135, 15));
                ImGui::Text(OBF("Login"));

                // Username
                ImGui::SetCursorPos(ImVec2(20, 50));
                ImGui::Text(OBF("Username:"));
                ImGui::SetCursorPos(ImVec2(20, 70));
                ImGui::PushItemWidth(280);
                ImGui::InputText("##username", UsernameBuffer, IM_ARRAYSIZE(UsernameBuffer));
                ImGui::PopItemWidth(); // Pop item width

                // Password
                ImGui::SetCursorPos(ImVec2(20, 100));
                ImGui::Text(OBF("Password:"));
                ImGui::SetCursorPos(ImVec2(20, 120));
                ImGui::PushItemWidth(280);
                ImGui::InputText("##password", PasswordBuffer, IM_ARRAYSIZE(PasswordBuffer), ImGuiInputTextFlags_Password);
                ImGui::PopItemWidth(); // Pop item width

                // Save credentials checkbox
                ImGui::SetCursorPos(ImVec2(20, 150));
                ImGui::Checkbox(OBF("Save Credentials"), &SaveCredentials);

                // Login button
                ImGui::SetCursorPos(ImVec2(170, 150));
                if (ImGui::Button(OBF("Login"), ImVec2(130, 30)) && !IsLoggingIn) {
                    IsLoggingIn = true;
                    LoginError = false;

                    std::string errorMsg;
                    if (AuthenticateUser(UsernameBuffer, PasswordBuffer, hwid, errorMsg)) {
                        IsLoggedIn = true;
                        UserName = UsernameBuffer;
                        UserExpiry = "Active";  // We'll need to get actual expiry from API

                        if (SaveCredentials) {
                            SaveLoginCredentials(UsernameBuffer, PasswordBuffer);
                        }
                    }
                    else {
                        LoginError = true;
                        LoginErrorMessage = errorMsg;
                    }

                    IsLoggingIn = false;
                }

                // Loading indicator
                if (IsLoggingIn) {
                    ImGui::SetCursorPos(ImVec2(140, 180));
                    ImGui::Text(OBF("Logging in..."));
                }

                // Error message
                if (LoginError) {
                    ImGui::SetCursorPos(ImVec2(20, 180));
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", LoginErrorMessage.c_str());
                }
            }
            ImGui::End();
        }
        else {
            // Main menu - Only show if logged in
            static WebsocketSubs SocketSubTab = Basic;
            static SettingsSubs MoreSubTab = Settings1;

            // Animation speed
            TabAnim = ImLerp(TabAnim, 0.0f, deltaTime * 10); // Animate towards 0

            ImGui::SetNextWindowSize(ImVec2(747, 635));
            ImGui::SetNextWindowBgAlpha(MenuAlpha);
            ImGui::Begin("ClanUtils", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            {
                
                auto draw = ImGui::GetWindowDrawList();
                auto pos = ImGui::GetWindowPos();
                auto size = ImGui::GetWindowSize();


                // Shadow/Glow effect logic specific to main menu
                ImVec4 brightRedVec = ImVec4(0.1f, 0.6f, 1.0f, MenuAlpha);
                ImVec4 deepRedVec = ImVec4(0.1f, 0.3f, 0.8f, MenuAlpha);

                static float time = 0.0f;
                time += deltaTime * 3.0f;

                float t = (sin(time) * 0.5f) + 0.5f;
                t = t * t * (3 - 2 * t);

                ImVec4 flickerColorVec = ImVec4(brightRedVec.x + (deepRedVec.x - brightRedVec.x) * t, brightRedVec.y + (deepRedVec.y - brightRedVec.y) * t, brightRedVec.z + (deepRedVec.z - brightRedVec.z) * t, MenuAlpha);
                ImColor flickerColor = ImColor(flickerColorVec.x, flickerColorVec.y, flickerColorVec.z, flickerColorVec.w);

                const float newWidth = 747 * 1.02f;
                const float newHeight = 635 * 1.02f;
                const float offsetX = (newWidth - 747) * 0.5f;
                const float offsetY = (newHeight - 635) * 0.5f;
                const float rounding = 10.0f;
                const ImDrawFlags flags = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight | ImDrawFlags_RoundCornersBottomLeft | ImDrawFlags_RoundCornersBottomRight;

                // Draw shadow
                ImGui::GetBackgroundDrawList()->AddShadowRect(ImVec2(pos.x - offsetX, pos.y - offsetY), ImVec2(pos.x + 747 + offsetX, pos.y + 635 + offsetY), flickerColor, 100, ImVec2(0, 0), flags, rounding);

                // Draw background rectangles
                draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 10), ImVec2(pos.x + 160, pos.y + 70), ImColor(0.05f, 0.05f, 0.05f, MenuAlpha), 5.0f, ImDrawFlags_RoundCornersAll);
                draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 80), ImVec2(pos.x + 160, pos.y + 555), ImColor(0.05f, 0.05f, 0.05f, MenuAlpha), 5.0f, ImDrawFlags_RoundCornersAll);
                draw->AddRectFilled(ImVec2(pos.x + 10, pos.y + 565), ImVec2(pos.x + 160, pos.y + 625), ImColor(0.05f, 0.05f, 0.05f, MenuAlpha), 5.0f, ImDrawFlags_RoundCornersAll);
                draw->AddRectFilled(ImVec2(pos.x + 170, pos.y + 10), ImVec2(pos.x + CHILD_X + FULL_CHILD_BASE_X, pos.y + 70), ImColor(0.05f, 0.05f, 0.05f, MenuAlpha), 5.0f, ImDrawFlags_RoundCornersAll);

                // Draw text labels
                draw->AddText(Fonts::Large, 22.0f, ImVec2(pos.x + 41, pos.y + 28), ImColor(60, 60, 255), OBF("Clan"));
                draw->AddText(Fonts::Large, 22.0f, ImVec2(pos.x + 88, pos.y + 28), ImColor(255, 255, 255), OBF("Utils"));
                draw->AddText(Fonts::Medium, 16.0f, ImVec2(pos.x + 597, pos.y + 30), ImColor(60, 60, 255), OBF(" discord.gg/"));
                draw->AddText(Fonts::Medium, 16.0f, ImVec2(pos.x + 680, pos.y + 30), ImColor(255, 255, 255), OBF("voidpg"));

                // Tab bar
                ImGui::SetCursorPos({ 20, 90 });
                ImGui::BeginGroup();
                {
                    if (ImGui::Tab(Tab == Websocket, ICON_FA_CODE, OBF("Websocket"), ImVec2(131, 32))) { Tab = Websocket; }
                    if (ImGui::Tab(Tab == Settings, ICON_FA_LIST, OBF("More"), ImVec2(131, 32))) { Tab = Settings; }
                }
                ImGui::EndGroup();

                if (PreviousTab != Tab)
                {
                    TabAnim = 20.0f; // Start animation from 20
                    PreviousTab = Tab;
                }

                switch (Tab)
                {
                    // TAB 1
                case Websocket:
                {
                    // Sub Tabs
                    ImGui::SetCursorPos({ 180, 20 });
                    ImGui::BeginGroup();
                    {
                        if (ImGui::SubTab(SocketSubTab == Basic, OBF("Basic"), ImVec2(100, 40))) { SocketSubTab = Basic; }
                        ImGui::SameLine();
                        if (ImGui::SubTab(SocketSubTab == Clan, OBF("Clan"), ImVec2(100, 40))) { SocketSubTab = Clan; }
                    }
                    ImGui::EndGroup();

                    // Content
                    switch (SocketSubTab)
                    {
                        // Sub 1
                    case Basic:
                    {
                        
                        // Column 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Currency"), ImVec2(HALF_CHILD_BASE_X, 185));
                        {
                            ImGui::Combo(OBF("Type"), &Variables::CurrencyType, Lists::CurrencyList.data(), Lists::CurrencyList.size(), 4);
                            ImGui::Text(OBF("Amount"));
                            ImGui::CustomInputInt(OBF("##currencyAmount"), "", &Variables::CurrencyAmount, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##addCurrency"), OBF("Add Currency"), QUART_BUTTON_LINE, 30)) { Variables::AddCurrency = true; }
                            ImGui::SameLine();
                            if (ImGui::CustomButton(OBF("##spendCurrency"), OBF("Spend Currency"), QUART_BUTTON_LINE, 30)) { Variables::SpendCurrency = true; }
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_X, 275 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Monthly Stats"), ImVec2(HALF_CHILD_BASE_X, 350));
                        {
                            ImGui::Text(OBF("Kills"));
                            ImGui::CustomInputInt(OBF("##kills"), "", &Variables::Kills, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            ImGui::Text(OBF("Deaths"));
                            ImGui::CustomInputInt(OBF("##deaths"), "", &Variables::Deaths, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            ImGui::Text(OBF("Head Shots"));
                            ImGui::CustomInputInt(OBF("##headShots"), "", &Variables::HeadShots, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            ImGui::Text(OBF("Kill Streak"));
                            ImGui::CustomInputInt(OBF("##killStreak"), "", &Variables::KillStreak, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            ImGui::Text(OBF("Win Streak"));
                            ImGui::CustomInputInt(OBF("##winStreak"), "", &Variables::WinStreak, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##updateStats"), OBF("Update Stats"), HALF_BUTTON_LINE, 45)) { Variables::MonthlyStats = true; }
                        }
                        ImGui::EndChild();

                        // Column 2
                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Consumables"), ImVec2(HALF_CHILD_BASE_X, 185));
                        {
                            ImGui::Combo(OBF("Type"), &Variables::ConsumableType, Lists::ConsumableList.data(), Lists::ConsumableList.size(), 4);
                            ImGui::Text(OBF("Amount"));
                            ImGui::CustomInputInt(OBF("##consumableAmount"), "", &Variables::ConsumableAmount, 9, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##addConsumable"), OBF("Add Consumable"), HALF_BUTTON_LINE, 30)) { Variables::AddConsumable = true; }
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 275 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Wins Adder"), ImVec2(HALF_CHILD_BASE_X, 215));
                        {
                            ImGui::Combo(OBF("Game Mode"), &Variables::GameMode, Lists::GameModesList.data(), Lists::GameModesList.size(), 6);
                            ImGui::Text(OBF("Amount"));
                            ImGui::CustomInputInt(OBF("##winsAmount"), "", &Variables::WinsAmount, 5, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##addWins"), OBF("Add Wins"), HALF_BUTTON_LINE, 30)) { Variables::AddWin = true; }
                            ImGui::Text(OBF("Do not use values over 1000 as it may\nfreeze/lag the game, use smaller ones."));
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 500 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Upgrade Clan"), ImVec2(HALF_CHILD_BASE_X, 125));
                        {
                            ImGui::CustomSliderInt(OBF("Level"), &Variables::ClanLevel, 2, 10, "%d", ImGuiSliderFlags_None);
                            if (ImGui::CustomButton(OBF("##setFort"), OBF("Set Fort Level"), QUART_BUTTON_LINE, 30)) { Variables::SetClanFort = true; }
                            ImGui::SameLine();
                            if (ImGui::CustomButton(OBF("##setTank"), OBF("Set Tank Level"), QUART_BUTTON_LINE, 30)) { Variables::SetClanTank = true; }
                            ImGui::Text(OBF("You must be either officer or leader"));
                        }
                        ImGui::EndChild();
                    }
                    break;

                    // Sub 2
                    case Clan:
                    {
                        // Column 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Account"), ImVec2(HALF_CHILD_BASE_X, 187));
                        {
                            ImGui::Combo(OBF("Clan Rank"), &Variables::ClanRank, Lists::ClanRanks.data(), Lists::ClanRanks.size(), 4);
                            ImGui::Text(OBF("Rank Points"));
                            ImGui::CustomInputInt(OBF("##rankPoints"), "", &Variables::RankPoints, 50, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##setClanRank"), OBF("Set Rank"), HALF_BUTTON_LINE, 30)) { Variables::SetRank = true; }
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_X, 277 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Modules"), ImVec2(HALF_CHILD_BASE_X, 257));
                        {
                            ImGui::Combo(OBF("Select Module"), &Variables::ModuleType, Lists::ModuleList.data(), Lists::ModuleList.size(), 8);
                            ImGui::Text(OBF("Module Amount"));
                            ImGui::CustomInputInt(OBF("##moduleAmount"), "", &Variables::ModuleAmount, 50, 0, ImGuiInputFlags_None, HALF_BUTTON_LINE);
                            if (ImGui::CustomButton(OBF("##addModule"), OBF("Add Module"), QUART_BUTTON_LINE, 30)) { Variables::AddModule = true; }
                            ImGui::SameLine();
                            if (ImGui::CustomButton(OBF("##addAllModule"), OBF("Add All"), QUART_BUTTON_LINE, 30)) { Variables::AddAllModules = true; }
                            if (ImGui::CustomButton(OBF("##upgradeModule"), OBF("Upgrade Module"), QUART_BUTTON_LINE, 30)) { Variables::CheckSlots[1] = true; }
                            ImGui::SameLine();
                            if (ImGui::CustomButton(OBF("##upgradeAllModules"), OBF("Upgrade All"), QUART_BUTTON_LINE, 30)) { Variables::CheckSlots[2] = true; }
                            ImGui::CustomSliderInt(OBF("Level Increaser"), &Variables::ModuleLevel, 1, 9, "%d", ImGuiSliderFlags_None);
                        }
                        ImGui::EndChild();

                        // Column 2
                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Clan Logo Editor"), ImVec2(HALF_CHILD_BASE_X, 135));
                        {
                            ImGui::Combo(OBF("Type"), &Variables::LogoType, Lists::LogoList.data(), Lists::LogoList.size(), 4);
                            if (ImGui::CustomButton(OBF("##editClan"), OBF("Edit Clan Logo"), HALF_BUTTON_LINE, 30)) { Variables::EditClanLogo = true; }
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 225 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Clan Name Changer"), ImVec2(HALF_CHILD_BASE_X, 170));
                        {
                            static ImVec4 selectedColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                            ImGui::Text(OBF("Clan Name"));
                            if (ImGui::ColorEdit3(OBF("##clannamecolorpicker"), (float*)&selectedColor)) {}
                            char colorCode[9];
                            snprintf(colorCode, sizeof(colorCode), "[%02X%02X%02X]", (int)(selectedColor.x * 255), (int)(selectedColor.y * 255), (int)(selectedColor.z * 255));
                            ImGui::CustomInputText(OBF("##clannamecolor"), OBF("Clan Name"), HALF_BUTTON_LINE, Variables::ClanNameColor, 5000, ImGuiInputFlags_None);
                            if (ImGui::CustomButton(OBF("##editClanName"), OBF("Edit Clan Name"), HALF_BUTTON_LINE, 30))
                            {
                                char finalClanName[21];
                                int remainingLength = 20 - (int)strlen(colorCode);
                                if (remainingLength > 0)
                                {
                                    snprintf(finalClanName, sizeof(finalClanName), "%s%.*s", colorCode, remainingLength, Variables::ClanNameColor);
                                }
                                else
                                {

                                    strncpy(finalClanName, colorCode, sizeof(finalClanName) - 1);
                                }
                                finalClanName[sizeof(finalClanName) - 1] = '\0';
                                strncpy(Variables::ClanNameColor, finalClanName, sizeof(Variables::ClanNameColor) - 1);
                                Variables::ClanNameColor[sizeof(Variables::ClanNameColor) - 1] = '\0';

                                Variables::EditClanName = true;
                            }
                            ImGui::Text(OBF("12 Letter Max"));
                        }
                        ImGui::EndChild();

                        ImGui::SetCursorPos(ImVec2(CHILD_SPACED_X, 405 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Level"), ImVec2(HALF_CHILD_BASE_X, 128));
                        {
                            ImGui::CustomSliderInt(OBF("Level"), &Variables::AdderLevel, 1, 65, "%d", ImGuiSliderFlags_None);
                            if (ImGui::CustomButton(OBF("##setLevel"), OBF("Set Level"), HALF_BUTTON_LINE, 30)) { Variables::SetLevel = true; }
                            ImGui::Text(OBF("Level Rewards after getting XP"));
                        }
                        ImGui::EndChild();

                        // Row 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 544 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Player Info"), ImVec2(FULL_CHILD_BASE_X, 80));
                        {
                            std::string PlayerLevelString = std::to_string(Variables::PlayerLevel);

                            ImGui::Text(OBF("Player ID: %s"), Variables::PlayerID.c_str());
                            ImGui::SameLine(); ImGui::SetCursorPosX(HALF_CHILD_BASE_X + 20);
                            ImGui::Text(OBF("Player Nick: %s"), Variables::PlayerUsername.c_str());
                            ImGui::Text(OBF("Player Level: %s"), PlayerLevelString.c_str());
                            ImGui::SameLine(); ImGui::SetCursorPosX(HALF_CHILD_BASE_X + 20);
                            ImGui::Text(OBF("Game Version: %s"), Variables::GameVersion.c_str());
                        }
                        ImGui::EndChild();
                    }
                    break;
                    }
                }
                break;

                // TAB 2
                case Settings:
                {
                    // Sub Tabs
                    ImGui::SetCursorPos({ 180, 20 });
                    ImGui::BeginGroup();
                    {
                        if (ImGui::SubTab(MoreSubTab == Settings1, OBF("Misc"), ImVec2(100, 40))) { MoreSubTab = Settings1; }
                        ImGui::SameLine();
                        if (ImGui::SubTab(MoreSubTab == Settings2, OBF("Test Features"), ImVec2(100, 40))) { MoreSubTab = Settings2; }
                        ImGui::SameLine();
                        if (ImGui::SubTab(MoreSubTab == Settings3, OBF("Test Features2"), ImVec2(100, 40))) { MoreSubTab = Settings3; }
                    }
                    ImGui::EndGroup();

                    // Content
                    switch (MoreSubTab)
                    {
                        // Sub 1
                    case Settings1:
                    {
                        // Row 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Socket"), ImVec2(FULL_CHILD_BASE_X, 290));
                        {
                            ImGui::Text(OBF("Event Name"));
                            ImGui::CustomInputText(OBF("##eventName"), OBF("Event Name"), FULL_BUTTON_LINE, Variables::EventInput, 100, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Command"));
                            ImGui::CustomInputText(OBF("##command"), OBF("Command"), FULL_BUTTON_LINE, Variables::CommandInput, 1000000, ImGuiInputFlags_None);
                            ImGui::Spacing();
                            if (ImGui::CustomButton(OBF("##sendCommand"), OBF("Send Command"), FULL_BUTTON_LINE, 30)) { Variables::SendCommand = true; }
                            if (ImGui::CustomButton(OBF("##reload"), OBF("Reload Socket"), FULL_BUTTON_LINE, 30)) { Variables::ReloadSocket = true; }
                            ImGui::Spacing();
                            ImGui::CustomCheckbox(OBF("Auto-Reload"), &Variables::AutoReload);
                            ImGui::CustomCheckbox(OBF("Log Websocket"), &Variables::LogWebsocket);
                            ImGui::Text(OBF("Do not use custom commands if you do not know what you are doing."));
                        }
                        ImGui::EndChild();

                        // Row 2
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 380 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Socials"), ImVec2(FULL_CHILD_BASE_X, 110));
                        {
                            if (ImGui::CustomButton(OBF("##yt"), OBF("YouTube"), FULL_BUTTON_LINE, 30)) { ShellExecuteA(NULL, "open", "https://youtube.com/@mrbeast", 0, 0, SW_SHOWNORMAL); }
                            if (ImGui::CustomButton(OBF("##dsc"), OBF("Discord"), FULL_BUTTON_LINE, 30)) { ShellExecuteA(NULL, "open", "https://discord.gg/voidpg", 0, 0, SW_SHOWNORMAL); }
                        }
                        ImGui::EndChild();

                        // Row 3
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 500 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Menu"), ImVec2(FULL_CHILD_BASE_X, 125));
                        {
                            ImGui::CustomCheckbox(OBF("Draw Ash"), &Variables::Snowflakes);
                            ImGui::CustomCheckbox(OBF("Background Dim"), &Variables::Background);
                            ImGui::SliderFloat(OBF("Dim Amount"), &Variables::DimAmmount, 0.f, 2.f);
                        }
                        ImGui::EndChild();
                    }
                    break;

                    // Sub 2
                    case Settings2:
                    {
                        // Row 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 80 - TabAnim));
                        ImGui::CustomChild(OBF("Info"), ImVec2(FULL_CHILD_BASE_X, 75));
                        {
                            ImGui::Text(OBF("Test Features, Working but will receive updates"));
                        }
                        ImGui::EndChild();  // Add this missing EndChild()

                        // Column 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 165 - TabAnim));
                        ImGui::CustomChild(OBF("Valor Generator"), ImVec2(HALF_CHILD_BASE_X, 187));
                        {
                            ImGui::CustomSliderInt(OBF("Valor Amount"), &Variables::ValorAmount, 1, 250, "%d", ImGuiSliderFlags_None);
                            ImGui::CustomSliderInt(OBF("Repeats"), &Variables::ValorRepeats, 1, 250, "%d", ImGuiSliderFlags_None);
                            if (ImGui::CustomButton(OBF("##addValor"), OBF("Try to Add Valor"), HALF_BUTTON_LINE, 30)) { Variables::AddValor = true; }
                            if (ImGui::CustomButton(OBF("##clanTaskCompleter"), OBF("Clan Tasks Completer"), HALF_BUTTON_LINE, 30)) { Variables::CompleteClanTasks = true; }
                        }
                        ImGui::EndChild();  // Add this missing EndChild()






                    }
                    break;
                    }
                    case Settings3:
                    {
                        // Row 1
                        ImGui::SetCursorPos(ImVec2(CHILD_X, 80 - TabAnim)); // Use TabAnim here
                        ImGui::CustomChild(OBF("Socket"), ImVec2(FULL_CHILD_BASE_X, 520));
                        {
                            ImGui::Text(OBF("Event Name"));
                            ImGui::CustomInputText(OBF("##eventName1"), OBF("Event Name1"), FULL_BUTTON_LINE, Variables::EventInput1, 100, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Event Name2"));
                            ImGui::CustomInputText(OBF("##eventName2"), OBF("Event Name2"), FULL_BUTTON_LINE, Variables::EventInput2, 100, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Event Name3"));
                            ImGui::CustomInputText(OBF("##eventName3"), OBF("Event Name3"), FULL_BUTTON_LINE, Variables::EventInput3, 100, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Command"));
                            ImGui::CustomInputText(OBF("##command1"), OBF("Command1"), FULL_BUTTON_LINE, Variables::CommandInput1, 1000000, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Command2"));
                            ImGui::CustomInputText(OBF("##command2"), OBF("Command2"), FULL_BUTTON_LINE, Variables::CommandInput2, 1000000, ImGuiInputFlags_None);
                            ImGui::Text(OBF("Command3"));
                            ImGui::CustomInputText(OBF("##command3"), OBF("Command3"), FULL_BUTTON_LINE, Variables::CommandInput3, 1000000, ImGuiInputFlags_None);
                            ImGui::Spacing();
                            if (ImGui::CustomButton(OBF("##sendCommand1"), OBF("Send Command1"), FULL_BUTTON_LINE, 30)) { Variables::SendCommand1 = true; }
                            if (ImGui::CustomButton(OBF("##sendCommand2"), OBF("Send Command2"), FULL_BUTTON_LINE, 30)) { Variables::SendCommand2 = true; }
                            if (ImGui::CustomButton(OBF("##sendCommand3"), OBF("Send Command3"), FULL_BUTTON_LINE, 30)) { Variables::SendCommand3 = true; }
                            if (ImGui::CustomButton(OBF("##sendSequence"), OBF("Send Sequence (1+2 then 3)"), FULL_BUTTON_LINE, 30))
                            {
                                Variables::SendCommand1 = true;
                                Variables::SendCommand2 = true;

                                // Set up a delayed send for command 3
                                std::thread([]() {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                                    Variables::SendCommand3 = true;
                                    }).detach();
                            }

                            if (ImGui::CustomButton(OBF("##reload"), OBF("Reload Socket"), FULL_BUTTON_LINE, 30)) { Variables::ReloadSocket = true; }
                            ImGui::Spacing();
                            ImGui::CustomCheckbox(OBF("Auto-Reload"), &Variables::AutoReload);
                            ImGui::CustomCheckbox(OBF("Log Websocket"), &Variables::LogWebsocket);
                            ImGui::Text(OBF("Do not use custom commands if you do not know what you are doing."));
                        }
                        ImGui::EndChild();
                    }
                    break;
                }
                break;
                }

                // Username and expiry info (Use actual logged-in username)
                std::string Expiry = UserExpiry; // Use the UserExpiry variable set during login
                draw->AddText(Fonts::Medium, 16.0f, ImVec2(pos.x + 27, pos.y + 580), ImColor(60, 60, 255), UserName.c_str()); // Use UserName variable
                draw->AddText(Fonts::Medium, 14.0f, ImVec2(pos.x + 27, pos.y + 600), ImColor(255, 255, 255), OBF("Expires:"));
                //draw->AddText(Fonts::Medium, 14.0f, ImVec2(pos.x + 76, pos.y + 600), ImColor(60, 60, 255), Expiry.c_str());
            }
            ImGui::End();
        }
    }

    // Notifs - Render outside the main menu alpha check if needed, or keep here if they should fade with menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(25.f / 255.f, 25.f / 255.f, 25.f / 255.f, 100.f / 255.f)); // Consider tying alpha to MenuAlpha or a separate notification alpha
    ImGui::RenderNotifications();
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(1);
    ImGui::EndFrame();
    ImGui::Render();

    // Set Render Target and Render ImGui Draw Data
    context->OMSetRenderTargets(1, &targetview, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}