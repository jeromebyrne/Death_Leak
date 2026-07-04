#ifndef DEATHLEAK_BACKENDS_MAC_MACFILESYSTEM_H
#define DEATHLEAK_BACKENDS_MAC_MACFILESYSTEM_H

#include "Engine/Platform/IFileSystem.h"

#include <filesystem>
#include <string>

class MacFileSystem final : public IFileSystem
{
public:
    explicit MacFileSystem(std::filesystem::path repoRoot);

    std::string AssetPath(const std::string& relativePath) const override;
    std::string SavePath(const std::string& relativePath) const override;
    std::string SettingsPath(const std::string& relativePath) const override;

private:
    std::filesystem::path AppSupportRoot() const;
    std::filesystem::path NormalizeRelativePath(const std::string& relativePath) const;

    std::filesystem::path mRepoRoot;
};

#endif
