#include "Backends/Mac/MacLaunchPaths.h"

#include <mach-o/dyld.h>

#include <vector>

namespace MacLaunchPaths
{
std::filesystem::path DetectRepoRoot()
{
    std::vector<char> buffer(4096, '\0');
    uint32_t size = static_cast<uint32_t>(buffer.size());
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
    {
        buffer.resize(size);
        if (_NSGetExecutablePath(buffer.data(), &size) != 0)
        {
            return std::filesystem::current_path();
        }
    }

    std::filesystem::path executablePath(buffer.data());
    std::filesystem::path current = executablePath.parent_path();

    for (int depth = 0; depth < 8 && !current.empty(); ++depth)
    {
        if (std::filesystem::exists(current / "Game" / "XmlFiles") ||
            std::filesystem::exists(current / "Game" / "src"))
        {
            return current;
        }

        current = current.parent_path();
    }

    return executablePath.parent_path();
}
}
