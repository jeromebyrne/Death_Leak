#include "Backends/Mac/MacGameBootstrap.h"
#include "Backends/Mac/MacLaunchPaths.h"

#include <filesystem>

int main()
{
    MacGameBootstrap bootstrap(MacLaunchPaths::DetectRepoRoot());
    return bootstrap.RunSmoke();
}
