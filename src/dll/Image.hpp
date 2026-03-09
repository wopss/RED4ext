#pragma once

#include <RED4ext/Api/v1/FileVer.hpp>
#include <RED4ext/Api/v1/SemVer.hpp>

#include <vector>

class Image
{
public:
    static Image* Get();

    bool IsCyberpunk() const;
    bool IsSupported() const;

    const RED4ext::v1::FileVer& GetFileVersion() const;
    const RED4ext::v1::SemVer& GetProductVersion() const;
    const std::vector<RED4ext::v1::FileVer> GetSupportedVersions() const;

private:
    Image();
    ~Image() = default;

    bool m_isCyberpunk;
    RED4ext::v1::FileVer m_fileVersion;
    RED4ext::v1::SemVer m_productVersion;
};
