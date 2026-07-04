#ifndef DEATHLEAK_WINDOWS_WINDOWSFILESYSTEM_H
#define DEATHLEAK_WINDOWS_WINDOWSFILESYSTEM_H

#include "Engine/Platform/IFileSystem.h"

#include <filesystem>
#include <string>

class WindowsFileSystem final : public IFileSystem
{
public:
    WindowsFileSystem();

    std::string AssetPath(const std::string& relativePath) const override;
    std::string SavePath(const std::string& relativePath) const override;
    std::string SettingsPath(const std::string& relativePath) const override;

private:
    std::filesystem::path ResolveBaseDirectory() const;
    std::filesystem::path NormalizeRelativePath(const std::string& relativePath) const;

    std::filesystem::path mBaseDirectory;
};

#endif
