#pragma once

#include <Unity/IL2CPP.hpp>
#include "Hooks.hpp"

namespace Cache
{
    size_t hash_pair(const std::pair<std::string, std::string>& p)
    {
        auto h1 = std::hash<std::string>{}(p.first);
        auto h2 = std::hash<std::string>{}(p.second);
        return h1 ^ (h2 << 1);
    }

    static std::unordered_map<std::string, IL2CPP::Image*> assemblies;
    static std::unordered_map<std::pair<std::string, std::string>, IL2CPP::Class*, decltype(&hash_pair)> classes{ 0, hash_pair };

    IL2CPP::Class* GetClass(const std::string& className, const std::string& assembly = "Assembly-CSharp.dll")
    {
        const auto key = std::make_pair(assembly, className);

        if (auto it = classes.find(key); it != classes.end())
        {
            return it->second;
        }

        auto& assemblyPtr = assemblies.try_emplace(assembly, IL2CPP::Domain().OpenAssembly(assembly)).first->second;
        auto klass = assemblyPtr->GetClass(className);

        classes[key] = klass;
        return klass;
    }
}

// Method Helpers
template<typename T>
LPVOID GetMethodPointer(const std::string& className, const T& methodIdentifier, const std::string& assembly = "Assembly-CSharp.dll")
{
    if (auto targetClass = Cache::GetClass(className, assembly))
    {
        if constexpr (std::is_integral_v<T>)
        {
            if (auto method = targetClass->GetMethod(methodIdentifier))
            {
                return method->GetPointer();
            }
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            if (auto method = targetClass->GetMethodByPattern(*methodIdentifier))
            {
                return method->GetPointer();
            }
        }
        else if constexpr (std::is_convertible_v<T, std::string>)
        {
            if (auto method = targetClass->GetMethod(methodIdentifier))
            {
                return method->GetPointer();
            }
        }
    }
    throw std::runtime_error("Failed to get method pointer");
}

LPVOID GetMappedPointer(std::string_view className, std::string_view methodName)
{
    if (auto klass = IL2CPP::ClassMapping::GetClass(className.data()))
    {
        if (auto method = klass->GetMethod(methodName.data()))
        {
            return method->GetPointer();
        }
    }
    throw std::runtime_error("Mapped class method not found");
}

LPVOID GetMappedPointer(std::string_view className, uint64_t index)
{
    if (auto klass = IL2CPP::ClassMapping::GetClass(className.data()))
    {
        if (auto method = klass->GetMethod(index))
        {
            return method->GetPointer();
        }
    }
    throw std::runtime_error("Mapped class method not found");
}

LPVOID GetMappedPointer(std::string_view className, IL2CPP::SignaturePattern* pattern)
{
    if (auto klass = IL2CPP::ClassMapping::GetClass(className.data()))
    {
        if (auto method = klass->GetMethodByPattern(*pattern))
        {
            return method->GetPointer();
        }
    }
    throw std::runtime_error("Mapped class method not found");
}

// Hook Helpers
template<typename Identifier>
void HookMethod(const std::string& className, const Identifier& methodIdentifier, LPVOID detour, LPVOID* original, const std::string& assembly = "Assembly-CSharp.dll")
{
    auto address = GetMethodPointer(className, methodIdentifier, assembly);
    AttachHook(address, detour, original);
}

template<typename Identifier>
void HookMappedMethod(std::string_view className, const Identifier& methodIdentifier, LPVOID detour, LPVOID* original)
{
    if (auto klass = IL2CPP::ClassMapping::GetClass(className.data()))
    {
        if constexpr (std::is_convertible_v<Identifier, std::string_view>)
        {
            if (auto method = klass->GetMethod(methodIdentifier))
            {
                AttachHook(method->GetPointer(), detour, original);
                return;
            }
        }
        else if constexpr (std::is_integral_v<Identifier>)
        {
            if (auto method = klass->GetMethod(methodIdentifier))
            {
                AttachHook(method->GetPointer(), detour, original);
                return;
            }
        }
        else if constexpr (std::is_pointer_v<Identifier>)
        {
            if (auto method = klass->GetMethodByPattern(*methodIdentifier))
            {
                AttachHook(method->GetPointer(), detour, original);
                return;
            }
        }
        throw std::runtime_error("Method not found in mapped class");
    }
    throw std::runtime_error("Mapped class not found");
}

void HookMethodOffset(uint64_t offset, LPVOID detour, LPVOID* original)
{
    static const auto moduleBase = reinterpret_cast<uint64_t>(GetModuleHandleA("GameAssembly.dll"));
    AttachHook(reinterpret_cast<LPVOID>(moduleBase + offset), detour, original);
}

// Field Helpers
template<typename T>
void SetField(IL2CPP::Object* instance, std::string_view fieldName, T value)
{
    if (!instance) throw std::invalid_argument("Null object instance");
    auto& field = instance->GetFieldRef<T>(fieldName.data());
    field = value;
}