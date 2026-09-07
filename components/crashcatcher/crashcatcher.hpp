#ifndef OPENMW_COMPONENTS_CRASHCATCHER_CRASHCATCHER_HPP
#define OPENMW_COMPONENTS_CRASHCATCHER_CRASHCATCHER_HPP

#include <filesystem>

<<<<<<< HEAD
#if (defined(__APPLE__) || (defined(__linux)  &&  !defined(ANDROID)) || (defined(__unix) &&  !defined(ANDROID)) || defined(__posix))
    #define USE_CRASH_CATCHER 0
=======
#if (defined(__APPLE__) || (defined(__linux) && !defined(ANDROID)) || (defined(__unix) && !defined(ANDROID))           \
    || defined(__posix))
void crashCatcherInstall(int argc, char** argv, const std::filesystem::path& crashLogPath);
>>>>>>> omw51
#else
inline void crashCatcherInstall(int /*argc*/, char** /*argv*/, const std::filesystem::path& /*crashLogPath*/) {}
#endif

#endif
