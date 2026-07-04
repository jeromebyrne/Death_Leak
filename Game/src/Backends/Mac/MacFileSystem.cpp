#include "Backends/Mac/MacFileSystem.h"

#include <cstdlib>

MacFileSystem::MacFileSystem(std::filesystem::path repoRoot):
    mRepoRoot(std::move(repoRoot))
{
}

std::string MacFileSystem::AssetPath(const std::string& relativePath) const
{
    return (mRepoRoot / "Game" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::string MacFileSystem::SavePath(const std::string& relativePath) const
{
    return (AppSupportRoot() / "Saves" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::string MacFileSystem::SettingsPath(const std::string& relativePath) const
{
    return (AppSupportRoot() / "Settings" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::filesystem::path MacFileSystem::AppSupportRoot() const
{
    const char* home = std::getenv("HOME");
    if (home == nullptr)
    {
        return mRepoRoot / "Game" / "UserData";
    }

    return std::filesystem::path(home) /
        "Library" /
        "Application Support" /
        "Death Leak";
}

std::filesystem::path MacFileSystem::NormalizeRelativePath(const std::string& relativePath) const
{
    std::string normalized = relativePath;
    for (char& c : normalized)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }

    return std::filesystem::path(normalized).relative_path();
}
