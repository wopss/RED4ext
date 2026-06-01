#pragma once

#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "Paths.hpp"

class Addresses
{
public:
    static void Construct(const Paths& aPaths);
    static Addresses* Instance();

    ~Addresses() = default;

    std::uintptr_t Resolve(std::uint32_t aHash) const;

private:
    Addresses(const Paths& aPaths);

    void LoadAddresses(const std::filesystem::path& aPath);
    void LoadSections();

    std::uint32_t m_codeOffset = 0;
    std::uint32_t m_dataOffset = 0;
    std::uint32_t m_rdataOffset = 0;
    std::unordered_map<std::uint32_t, std::uintptr_t> m_addresses;
};
