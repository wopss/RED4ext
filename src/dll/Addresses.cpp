#include "Addresses.hpp"

#include <ios>
#include <string>
#include <string_view>

#include <RED4ext/Relocation.hpp>
#include <spdlog/spdlog.h>

#include "Utils.hpp"

namespace
{
std::unique_ptr<Addresses> g_addresses;
}

Addresses::Addresses(const Paths& aPaths)
{
    constexpr auto filename = L"cyberpunk2077_addresses.json";
    auto filePath = aPaths.GetX64Dir() / filename;

    LoadSections();
    LoadAddresses(filePath);
}

void Addresses::Construct(const Paths& aPaths)
{
    g_addresses.reset(new Addresses(aPaths));
}

Addresses* Addresses::Instance()
{
    return g_addresses.get();
}

std::uintptr_t Addresses::Resolve(std::uint32_t aHash) const
{
    const auto it = m_addresses.find(aHash);
    if (it == m_addresses.end())
    {
        return 0;
    }

    const auto address = it->second;
    return address;
}

void Addresses::LoadAddresses(const std::filesystem::path& aPath)
{
    if (!exists(aPath))
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"The addresses JSON does not exists\n\nPath: {}", aPath);
        return;
    }

    spdlog::info(L"Loading game's addresses from '{}'...", aPath);

    simdjson::ondemand::parser parser;
    simdjson::padded_string json = simdjson::padded_string::load(aPath.string());
    simdjson::ondemand::document document = parser.iterate(json);

    simdjson::ondemand::array root;
    auto error = document["Addresses"].get_array().get(root);
    if (error)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"Could not get the root array for the addresses: {}",
                                            Utils::Widen(simdjson::error_message(error)));
        return;
    }

    auto base = reinterpret_cast<std::uintptr_t>(GetModuleHandle(nullptr));

    root.reset();

    for (auto entry : root)
    {
        auto hashField = entry.find_field("hash");
        auto offsetField = entry.find_field("offset");

        if (!hashField.error() && !offsetField.error())
        {
            std::uint64_t hash;
            error = hashField.get_uint64_in_string().get(hash);
            if (error)
            {
                SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"Could not get the hash for an address: {}",
                                                    Utils::Widen(simdjson::error_message(error)));
                return;
            }

            std::string_view offsetStr;
            error = offsetField.get_string().get(offsetStr);
            if (error)
            {
                SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"Could not get the offset for an address: {}",
                                                    Utils::Widen(simdjson::error_message(error)));
                return;
            }

            std::stringstream stream;
            stream << offsetStr;

            std::uint32_t segment;
            char separator;
            std::uint32_t offset;
            stream >> std::hex >> segment >> separator >> offset;

            switch (segment)
            {
            case 1:
                offset += m_codeOffset;
                break;
            case 2:
                offset += m_rdataOffset;
                break;
            case 3:
                offset += m_dataOffset;
                break;
            }

            auto address = offset + base;
            m_addresses.emplace(static_cast<std::uint32_t>(hash), address);
        }
    }

    spdlog::info("{} game addresses loaded", m_addresses.size());
}

void Addresses::LoadSections()
{
    HMODULE hModule = GetModuleHandle(NULL);
    if (hModule == NULL)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"Error: Could not get module handle.");
        return;
    }

    // Access the DOS header
    IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)hModule;
    // Access the PE header
    IMAGE_NT_HEADERS* peHeader = (IMAGE_NT_HEADERS*)((BYTE*)hModule + dosHeader->e_lfanew);

    // Check for PE signature
    if (peHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        SHOW_MESSAGE_BOX_AND_EXIT_FILE_LINE(L"Error: PE signature not found.");
        return;
    }

    // Access the section headers
    IMAGE_SECTION_HEADER* sectionHeaders = IMAGE_FIRST_SECTION(peHeader);
    const int numberOfSections = peHeader->FileHeader.NumberOfSections;

    // List the sections
    for (int i = 0; i < numberOfSections; i++)
    {
        IMAGE_SECTION_HEADER* sectionHeader = &sectionHeaders[i];
        if (strcmp(reinterpret_cast<const char*>(sectionHeader->Name), ".text") == 0)
            m_codeOffset = sectionHeader->VirtualAddress;
        else if (strcmp(reinterpret_cast<const char*>(sectionHeader->Name), ".data") == 0)
            m_dataOffset = sectionHeader->VirtualAddress;
        else if (strcmp(reinterpret_cast<const char*>(sectionHeader->Name), ".rdata") == 0)
            m_rdataOffset = sectionHeader->VirtualAddress;
    }
}

RED4EXT_C_EXPORT std::uintptr_t RED4EXT_CALL RED4ext_ResolveAddress(const std::uint32_t aHash)
{
    return Addresses::Instance()->Resolve(aHash);
}
