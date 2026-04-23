#pragma once
#include <Unity/IL2CPP.hpp>

namespace Structs
{
    namespace Socket
    {
        struct Packet
        {
            IL2CPP::String* Placeholder;
            LPVOID          TransportEventTypes;
            LPVOID          SocketIOEventTypes;
            INT             AttachmentCount;
            INT             Id;
            IL2CPP::String* Namespace;
            IL2CPP::String* Payload;
            IL2CPP::String* EventName;
            LPVOID          byte;
            BOOL            IsDecoded;
            IL2CPP::Object* DecodedArgs;
        };

        struct Response
        {
            nlohmann::json                request;
            nlohmann::json                response;
            std::string                   ResponseStatus;
            std::function<VOID(Response)> OnRequestReceived;
            BOOL                          ForceExecute = false;
        };
        inline std::vector<Response> Responses;
    }
}