#ifndef DEATHLEAK_BACKENDS_MAC_MACLAUNCHPATHS_H
#define DEATHLEAK_BACKENDS_MAC_MACLAUNCHPATHS_H

#include <filesystem>

namespace MacLaunchPaths
{
std::filesystem::path DetectRepoRoot();
}

#endif
