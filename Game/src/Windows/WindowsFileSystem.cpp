#include "Windows/WindowsFileSystem.h"

#include <windows.h>

#include <cstdlib>

WindowsFileSystem::WindowsFileSystem():
    mBaseDirectory(ResolveBaseDirectory())
{
}

std::string WindowsFileSystem::AssetPath(const std::string& relativePath) const
{
    return (mBaseDirectory / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::string WindowsFileSystem::SavePath(const std::string& relativePath) const
{
    return (mBaseDirectory / "UserData" / "Saves" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::string WindowsFileSystem::SettingsPath(const std::string& relativePath) const
{
    return (mBaseDirectory / "UserData" / "Settings" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::filesystem::path WindowsFileSystem::ResolveBaseDirectory() const
{
    wchar_t buffer[MAX_PATH] = {0};
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        return std::filesystem::current_path();
    }

    return std::filesystem::path(buffer).parent_path();
}

std::filesystem::path WindowsFileSystem::NormalizeRelativePath(const std::string& relativePath) const
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
