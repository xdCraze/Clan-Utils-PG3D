#pragma once
#include <Obfusheader.hpp>
using json = nlohmann::ordered_json;

namespace Socket
{
    // Random req_id
    unsigned long Random(int digits = 9)
    {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());

        unsigned long long min = std::pow(10, digits - 1);
        unsigned long long max = std::pow(10, digits) - 1;
        std::uniform_int_distribution<unsigned long long> dist(min, max);

        unsigned long long randomNumber = dist(gen);
        return randomNumber;
    }

    // Command Creation
    IL2CPP::Array<IL2CPP::Object*>* CreateSocketCommand(const json& Data)
    {
        IL2CPP::Array<IL2CPP::Object*>* SocketCommand = IL2CPP::Array<IL2CPP::Object*>::Create<IL2CPP::Object*>(const_cast<IL2CPP::Class*>(IL2CPP::DefaultTypeClass::Object), 1);
        SocketCommand->GetVectorPointer()[0] = Pointers::Json::Decode(IL2CPP::String::Create(Data.dump().c_str()));
        return SocketCommand;
    }

    // Logging
    void LogSocketEvent(const std::string& EventName, const std::string& Data, const bool& isSend)
    {
        if (Variables::LogWebsocket)
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, isSend ? FOREGROUND_BLUE : 14);
            std::cout << OBF("[ SOCKET ] ") << EventName << (isSend ? OBF(" -> ") : OBF(" <- "));
            SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
            std::cout << Data << "\n";
        }
    }

    VOID(*Emit_o)(IL2CPP::Object* Instance, IL2CPP::String* eventName, IL2CPP::Array<IL2CPP::Object*>* args);
    VOID Emit(IL2CPP::Object* Instance, IL2CPP::String* eventName, IL2CPP::Array<IL2CPP::Object*>* args)
    {
        // Early Return
        if (Instance == nullptr) return;
        Pointers::SocketInstance = Instance;

        std::string EventName = eventName->ToString();
        std::string DecodedArgs = Pointers::Json::Encode((IL2CPP::Object*)args)->ToString();
        json Data = json::parse(DecodedArgs);

        // Parsing
        if (EventName == OBF("get_info_clan_warV2"))
        {
            Variables::ClanID = Data[0][OBF("clan_id")];
            Variables::ClanWarID = Data[0][OBF("war_id")];
        }

        // Log Event
        LogSocketEvent(EventName, DecodedArgs, true);
        Emit_o(Instance, eventName, args);
    }

    VOID(*OnPacket_o)(IL2CPP::Object* Instance, Structs::Socket::Packet* packet);
    VOID OnPacket(IL2CPP::Object* Instance, Structs::Socket::Packet* packet)
    {
        OnPacket_o(Instance, packet);

        if (packet->EventName && packet->IsDecoded)
        {
            std::string EventName = packet->EventName->ToString();
            std::string DecodedArgs = Pointers::Json::Encode(packet->DecodedArgs)->ToString();
            json Data = json::parse(DecodedArgs);

            // On Request Received
            int req_id = Data[0][OBF("req_id")].is_null() ? -1 : Data[0][OBF("req_id")].get<int>();
            if (req_id != -1)
            {
                auto it = std::find_if(Structs::Socket::Responses.begin(), Structs::Socket::Responses.end(), [req_id](const Structs::Socket::Response& response) { return response.request[OBF("req_id")].get<int>() == req_id; });

                if (it != Structs::Socket::Responses.end())
                {
                    Structs::Socket::Response& data = *it;
                    data.response = Data;
                    json status = Data[0][OBF("status")];
                    std::string fail = OBF("FAIL"), ok = OBF("OK");
                    data.ResponseStatus = ok;

                    if (!status.is_null() && status.get<std::string>() != OBF("ok"))
                    {
                        data.ResponseStatus = fail;
                        if (!data.ForceExecute) data.OnRequestReceived = nullptr;
                    }

                    if (data.OnRequestReceived != nullptr) data.OnRequestReceived(data);
                    Structs::Socket::Responses.erase(it);
                }
            }

            // Process Events
            if (EventName == OBF("get_progress"))
            {
                if (Data[0].contains(OBF("slots")) && Data[0][OBF("slots")].is_object() && !Data[0][OBF("slots")].empty())
                {
                    json& slots = Data[0][OBF("slots")];

                    // Slots Helpers
                    auto ObjectSlotData = [&](const std::string& slot, const std::function<void(json&)>& updateFn)
                    {
                        if (slots.contains(slot) && slots[slot].is_object()) updateFn(slots[slot]);
                    };

                    auto ArraySlotDataFromKey = [&](const std::string& slot, const std::string& field, const std::function<void(json&)>& updateFn)
                    {
                        if (slots.contains(slot) && slots[slot].is_object() && slots[slot].contains(field))
                        {
                            auto& array = slots[slot][field];
                            if (array.is_array() && !array.empty()) { updateFn(array); }
                        }
                    };

                    // My Progress Slots
                    ObjectSlotData(OBF("37"), [](json& data) { Variables::ModuleSlot = data; });
                    ArraySlotDataFromKey(OBF("56"), OBF("t"), [](json& data) { Variables::TutorialSlot = data; });
                }
            }

            // Log Event
            LogSocketEvent(EventName, DecodedArgs, false);
        }
    }

    // Send Command
    int SendCommand(json data, const std::function<void(Structs::Socket::Response)>& OnRequestReceived = nullptr, const bool& ForceExecute = true)
    {
        if (!data.is_array()) return -1;
        int req_id = Random();
        data[1][OBF("req_id")] = req_id;

        Structs::Socket::Response ResponseData;
        ResponseData.request = data[1];
        if (OnRequestReceived != nullptr) ResponseData.OnRequestReceived = OnRequestReceived;
        ResponseData.ForceExecute = ForceExecute;

        Structs::Socket::Responses.emplace_back(ResponseData);
        Emit(Pointers::SocketInstance, IL2CPP::String::Create(data[0].get<std::string>().c_str()), CreateSocketCommand(data[1]));

        return req_id;
    }
    int SendCommand1(json data, const std::function<void(Structs::Socket::Response)>& OnRequestReceived = nullptr, const bool& ForceExecute = true)
    {
        if (!data.is_array()) return -1;
        int req_id = Random();
        data[1][OBF("req_id")] = req_id;

        Structs::Socket::Response ResponseData;
        ResponseData.request = data[1];
        if (OnRequestReceived != nullptr) ResponseData.OnRequestReceived = OnRequestReceived;
        ResponseData.ForceExecute = ForceExecute;

        Structs::Socket::Responses.emplace_back(ResponseData);
        Emit(Pointers::SocketInstance, IL2CPP::String::Create(data[0].get<std::string>().c_str()), CreateSocketCommand(data[1]));

        return req_id;
    }
    int SendCommand2(json data, const std::function<void(Structs::Socket::Response)>& OnRequestReceived = nullptr, const bool& ForceExecute = true)
    {
        if (!data.is_array()) return -1;
        int req_id = Random();
        data[1][OBF("req_id")] = req_id;

        Structs::Socket::Response ResponseData;
        ResponseData.request = data[1];
        if (OnRequestReceived != nullptr) ResponseData.OnRequestReceived = OnRequestReceived;
        ResponseData.ForceExecute = ForceExecute;

        Structs::Socket::Responses.emplace_back(ResponseData);
        Emit(Pointers::SocketInstance, IL2CPP::String::Create(data[0].get<std::string>().c_str()), CreateSocketCommand(data[1]));

        return req_id;
    }
    int SendCommand3(json data, const std::function<void(Structs::Socket::Response)>& OnRequestReceived = nullptr, const bool& ForceExecute = true)
    {
        if (!data.is_array()) return -1;
        int req_id = Random();
        data[1][OBF("req_id")] = req_id;

        Structs::Socket::Response ResponseData;
        ResponseData.request = data[1];
        if (OnRequestReceived != nullptr) ResponseData.OnRequestReceived = OnRequestReceived;
        ResponseData.ForceExecute = ForceExecute;

        Structs::Socket::Responses.emplace_back(ResponseData);
        Emit(Pointers::SocketInstance, IL2CPP::String::Create(data[0].get<std::string>().c_str()), CreateSocketCommand(data[1]));

        return req_id;
    }
    // Send Notif Command
    void SendNotifCommand(const std::string& requestName, const json& command, const bool& autoReload = true)
    {
        ImGui::InsertNotification({ ImGuiToastType::Info, 3000, (OBF("Sent Request: ") + requestName).c_str() });
        SendCommand(command, [requestName, autoReload](Structs::Socket::Response packet)
        {
            ImGuiToastType toastType = (packet.ResponseStatus == OBF("OK")) ? ImGuiToastType::Success : ImGuiToastType::Error;
            if (packet.ResponseStatus == OBF("OK") && autoReload)
            {
                Variables::ReloadSocket = Variables::AutoReload;
            }
            const std::string message = OBF("Received Request: ") + requestName + OBF(" | Status: ") + packet.ResponseStatus;
            ImGui::InsertNotification({ toastType, 3000, message.c_str() });
        });
    }

    // Init
    void Init()
    {
        HookMethod("BestHTTP.SocketIO.Socket", "Emit", Emit, (LPVOID*)&Emit_o);
        HookMethod("BestHTTP.SocketIO.Socket", "BestHTTP.SocketIO.ISocket.OnPacket", OnPacket, (LPVOID*)&OnPacket_o);
    }
}