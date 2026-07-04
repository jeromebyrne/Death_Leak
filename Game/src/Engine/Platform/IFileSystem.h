#ifndef DEATHLEAK_ENGINE_PLATFORM_IFILESYSTEM_H
#define DEATHLEAK_ENGINE_PLATFORM_IFILESYSTEM_H

#include <string>

class IFileSystem
{
public:
    virtual ~IFileSystem() = default;

    virtual std::string AssetPath(const std::string& relativePath) const = 0;
    virtual std::string SavePath(const std::string& relativePath) const = 0;
    virtual std::string SettingsPath(const std::string& relativePath) const = 0;
};

#endif
