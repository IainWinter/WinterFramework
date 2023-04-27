#include "io/LibraryName.h"

#ifdef _WIN32 // Windows
#   define LIB_EXTENSION ".dll"
#   define LIB_PREFIX ""
#   define DEBUG_LIB_SUFFIX "d"
#elif __APPLE__ // macOS
#   define LIB_EXTENSION ".dylib"
#   define LIB_PREFIX "lib"
#   define DEBUG_LIB_SUFFIX ""
#else // Linux and other Unix-like systems
#   define LIB_EXTENSION ".so"
#   define LIB_PREFIX "lib"
#   define DEBUG_LIB_SUFFIX ""
#endif

std::string GetLibraryName(const std::string& libName)
{
#ifdef NDEBUG // release mode
    std::string libPath = LIB_PREFIX + libName + LIB_EXTENSION;
#else // debug mode
    std::string libPath = LIB_PREFIX + libName + DEBUG_LIB_SUFFIX + LIB_EXTENSION;
#endif

    return libPath;
}
