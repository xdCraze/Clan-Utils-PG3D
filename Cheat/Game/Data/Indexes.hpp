#pragma once

namespace Indexes
{
    // Pointers
    inline const uint64_t GetID = 0x2;
    inline const uint64_t GetUsername = 0x34; // Search "ProfileController :" -> "public Void TurnOn()" -> 1 Above this
    inline const uint64_t GetLevel = 0x6; // Search "public sealed class ExperienceController : MonoBehaviour" -> 7th offset below
}