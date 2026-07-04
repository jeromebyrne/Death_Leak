#include "Backends/Mac/MacFileSystem.h"

#include <CoreFoundation/CoreFoundation.h>

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <vector>

namespace
{
std::filesystem::path ResolveCaseInsensitivePath(const std::filesystem::path& base, const std::filesystem::path& relative)
{
    std::filesystem::path current = base;
    if (!std::filesystem::exists(current))
    {
        return base / relative;
    }

    for (const auto& part : relative)
    {
        const std::string wanted = part.string();
        if (wanted.empty() || wanted == ".")
        {
            continue;
        }

        const std::filesystem::path exact = current / part;
        if (std::filesystem::exists(exact))
        {
            current = exact;
            continue;
        }

        std::string wantedLower = wanted;
        std::transform(wantedLower.begin(), wantedLower.end(), wantedLower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        bool found = false;
        for (const auto& entry : std::filesystem::directory_iterator(current))
        {
            std::string candidate = entry.path().filename().string();
            std::transform(candidate.begin(), candidate.end(), candidate.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });

            if (candidate == wantedLower)
            {
                current = entry.path();
                found = true;
                break;
            }
        }

        if (!found)
        {
            return base / relative;
        }
    }

    return current;
}

std::filesystem::path ResolveCompatibilityAssetPath(const std::filesystem::path& normalizedRelativePath)
{
    static const std::vector<std::pair<std::string, std::string>> kAliases = {
        {"Media/objects/saw.png", "Media/objects/windmill_blade.png"},
        {"Media/controls_crate.png", "Media/crate/normal.png"},
        {"Media/ground/bounce_test.png", "Media/crate/normal.png"},
        {"Media/ground/noise_test.png", "Media/UI/noise_layer.png"},
        {"Media/test_audio_object.png", "Media/editor/audio.png"},
        {"XmlFiles/test_smashable.xml", "XmlFiles/stone_smashable.xml"},
    };

    const std::string normalized = normalizedRelativePath.generic_string();
    for (const auto& alias : kAliases)
    {
        if (normalized == alias.first)
        {
            return std::filesystem::path(alias.second);
        }
    }

    return normalizedRelativePath;
}
}

MacFileSystem::MacFileSystem(std::filesystem::path repoRoot):
    mRepoRoot(std::move(repoRoot))
{
}

std::string MacFileSystem::AssetPath(const std::string& relativePath) const
{
    const std::filesystem::path normalized = ResolveCompatibilityAssetPath(NormalizeRelativePath(relativePath));

    const std::filesystem::path bundleRoot = BundleResourcesRoot();
    if (!bundleRoot.empty())
    {
        const std::filesystem::path bundled = ResolveCaseInsensitivePath(bundleRoot, normalized);
        if (std::filesystem::exists(bundled))
        {
            return bundled.lexically_normal().string();
        }
    }

    const std::filesystem::path gameRoot = mRepoRoot / "Game";
    return ResolveCaseInsensitivePath(gameRoot, normalized).lexically_normal().string();
}

std::string MacFileSystem::SavePath(const std::string& relativePath) const
{
    return (AppSupportRoot() / "Saves" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::string MacFileSystem::SettingsPath(const std::string& relativePath) const
{
    return (AppSupportRoot() / "Settings" / NormalizeRelativePath(relativePath)).lexically_normal().string();
}

std::filesystem::path MacFileSystem::BundleResourcesRoot() const
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle == nullptr)
    {
        return {};
    }

    CFURLRef resourcesUrl = CFBundleCopyResourcesDirectoryURL(bundle);
    if (resourcesUrl == nullptr)
    {
        return {};
    }

    char buffer[PATH_MAX] = {0};
    const Boolean ok = CFURLGetFileSystemRepresentation(resourcesUrl, true, reinterpret_cast<UInt8*>(buffer), PATH_MAX);
    CFRelease(resourcesUrl);
    if (!ok)
    {
        return {};
    }

    return std::filesystem::path(buffer);
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
