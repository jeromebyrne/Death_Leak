#include "Backends/Mac/MacGameBootstrap.h"

#include <filesystem>

int main()
{
    MacGameBootstrap bootstrap(std::filesystem::current_path());
    return bootstrap.RunSmoke();
}
