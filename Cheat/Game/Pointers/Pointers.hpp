#pragma once

namespace Pointers
{
	// Dummy Pointers
	IL2CPP::Object* SocketInstance = nullptr;

    // Assemblies
	constexpr const char* CoreModule = "UnityEngine.CoreModule.dll";

	// Pointer Wrapper
	namespace Json
	{
		IL2CPP::Wrapper::Method<IL2CPP::Object* (IL2CPP::String*)> Decode;
		IL2CPP::Wrapper::Method<IL2CPP::String* (IL2CPP::Object*)> Encode;
	}

	namespace Profile
	{
		IL2CPP::Wrapper::Method<IL2CPP::String* ()> GetVersion, GetID, GetUsername;
		IL2CPP::Wrapper::Method<int()> GetLevel;
	}

	// Initializate
	void Init()
	{
		// JSON methods
		{
			Json::Decode = GetMethodPointer("BestHTTP.JSON.Json", "Decode");
			Json::Encode = GetMethodPointer("BestHTTP.JSON.Json", "Encode");
		}

		// Profile methods
		{
			Profile::GetVersion = GetMethodPointer("UnityEngine.Application", "get_version", CoreModule);
			Profile::GetID = GetMappedPointer("ProfileClass", Indexes::GetID);
			Profile::GetUsername = GetMethodPointer("ProfileController", Indexes::GetUsername);
			Profile::GetLevel = GetMethodPointer("ExperienceController", Indexes::GetLevel);
		}
	}
}