#include <components/debug/debugging.hpp>
#include <components/fallback/fallback.hpp>
#include <components/fallback/validate.hpp>
#include <components/files/configurationmanager.hpp>
#include <components/misc/osgpluginchecker.hpp>
#include <components/misc/rng.hpp>
#include <components/platform/platform.hpp>
#include <components/version/version.hpp>

#include "mwgui/debugwindow.hpp"

#include "engine.hpp"
#include "options.hpp"

#include <boost/program_options/variables_map.hpp>

/*
    Start of tes3mp addition

    Include the header of the multiplayer's Main class
*/
#include "mwmp/Main.hpp"
/*
    End of tes3mp addition
*/

#if defined(_WIN32)
#include <components/misc/windows.hpp>
// makes __argc and __argv available on windows
#include <cstdlib>

extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

#include <filesystem>

#if (defined(__APPLE__) || defined(__linux) || defined(__unix) || defined(__posix))
#include <unistd.h>
#endif

<<<<<<< HEAD
/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include <components/openmw-mp/ErrorMessages.hpp>
#include <components/openmw-mp/TimedLog.hpp>
#include <components/openmw-mp/Utils.hpp>
#include <components/openmw-mp/Version.hpp>
/*
    End of tes3mp addition
*/


using namespace Fallback;

=======
>>>>>>> omw51
/**
 * \brief Parses application command line and calls \ref Cfg::ConfigurationManager
 * to parse configuration files.
 *
 * Results are directly written to \ref Engine class.
 *
 * \retval true - Everything goes OK
 * \retval false - Error
 */
bool parseOptions(int argc, char** argv, OMW::Engine& engine, Files::ConfigurationManager& cfgMgr)
{
    // Create a local alias for brevity
    namespace bpo = boost::program_options;
    typedef std::vector<std::string> StringsVector;

<<<<<<< HEAD
    bpo::options_description desc("Syntax: openmw <options>\nAllowed options");

    desc.add_options()
        ("help", "print help message")
        ("version", "print version information and quit")

        ("replace", bpo::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "")
            ->multitoken()->composing(), "settings where the values from the current source should replace those from lower-priority sources instead of being appended")

        ("data", bpo::value<Files::EscapePathContainer>()->default_value(Files::EscapePathContainer(), "data")
            ->multitoken()->composing(), "set data directories (later directories have higher priority)")

        ("data-local", bpo::value<Files::EscapePath>()->default_value(Files::EscapePath(), ""),
            "set local data directory (highest priority)")

        ("fallback-archive", bpo::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "fallback-archive")
            ->multitoken()->composing(), "set fallback BSA archives (later archives have higher priority)")

        ("resources", bpo::value<Files::EscapePath>()->default_value(Files::EscapePath(), "resources"),
            "set resources directory")

        ("start", bpo::value<Files::EscapeHashString>()->default_value(""),
            "set initial cell")

        ("content", bpo::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "")
            ->multitoken()->composing(), "content file(s): esm/esp, or omwgame/omwaddon")

        ("groundcover", bpo::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "")
            ->multitoken()->composing(), "groundcover content file(s): esm/esp, or omwgame/omwaddon")

        ("no-sound", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "disable all sounds")

        ("script-all", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "compile all scripts (excluding dialogue scripts) at startup")

        ("script-all-dialogue", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "compile all dialogue scripts at startup")

        ("script-console", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "enable console-only script functionality")

        ("script-run", bpo::value<Files::EscapeHashString>()->default_value(""),
            "select a file containing a list of console commands that is executed on startup")

        ("script-warn", bpo::value<int>()->implicit_value (1)
            ->default_value (1),
            "handling of warnings when compiling scripts\n"
            "\t0 - ignore warning\n"
            "\t1 - show warning but consider script as correctly compiled anyway\n"
            "\t2 - treat warnings as errors")

        ("script-blacklist", bpo::value<Files::EscapeStringVector>()->default_value(Files::EscapeStringVector(), "")
            ->multitoken()->composing(), "ignore the specified script (if the use of the blacklist is enabled)")

        ("script-blacklist-use", bpo::value<bool>()->implicit_value(true)
            ->default_value(true), "enable script blacklisting")

        ("load-savegame", bpo::value<Files::EscapePath>()->default_value(Files::EscapePath(), ""),
            "load a save game file on game startup (specify an absolute filename or a filename relative to the current working directory)")

        ("skip-menu", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "skip main menu on game startup")

        ("new-game", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "run new game sequence (ignored if skip-menu=0)")

        ("fs-strict", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "strict file system handling (no case folding)")

        ("encoding", bpo::value<Files::EscapeHashString>()->
            default_value("win1252"),
            "Character encoding used in OpenMW game messages:\n"
            "\n\twin1250 - Central and Eastern European such as Polish, Czech, Slovak, Hungarian, Slovene, Bosnian, Croatian, Serbian (Latin script), Romanian and Albanian languages\n"
            "\n\twin1251 - Cyrillic alphabet such as Russian, Bulgarian, Serbian Cyrillic and other languages\n"
            "\n\twin1252 - Western European (Latin) alphabet, used by default")

        ("fallback", bpo::value<FallbackMap>()->default_value(FallbackMap(), "")
            ->multitoken()->composing(), "fallback values")

        ("no-grab", bpo::value<bool>()->implicit_value(true)->default_value(false), "Don't grab mouse cursor")

        ("export-fonts", bpo::value<bool>()->implicit_value(true)
            ->default_value(false), "Export Morrowind .fnt fonts to PNG image and XML file in current directory")

        ("activate-dist", bpo::value <int> ()->default_value (-1), "activation distance override")

        ("random-seed", bpo::value <unsigned int> ()
            ->default_value(Misc::Rng::generateDefaultSeed()),
            "seed value for random number generator")
    ;

    /*
        Start of tes3mp addition

        Parse options added by multiplayer
    */
    mwmp::Main::optionsDesc(&desc);
    /*
        End of tes3mp addition
    */

    bpo::parsed_options valid_opts = bpo::command_line_parser(argc, argv)
        .options(desc).allow_unregistered().run();

=======
    bpo::options_description desc = OpenMW::makeOptionsDescription();
>>>>>>> omw51
    bpo::variables_map variables;

    Files::parseArgs(argc, argv, variables, desc);
    bpo::notify(variables);

    if (variables.count("help"))
    {
        Debug::getRawStdout() << desc << std::endl;
        return false;
    }

    if (variables.count("version"))
    {
        Debug::getRawStdout() << Version::getOpenmwVersionDescription() << std::endl;
        return false;
    }

    cfgMgr.processPaths(variables, std::filesystem::current_path());

    cfgMgr.readConfiguration(variables, desc);

<<<<<<< HEAD
    Version::Version v = Version::getOpenmwVersion(variables["resources"].as<Files::EscapePath>().mPath.string());

    /*
        Start of tes3mp addition

        Print the multiplayer version first
    */
    Log(Debug::Info) << Utils::getVersionInfo("TES3MP client", TES3MP_VERSION, v.mCommitHash, TES3MP_PROTO_VERSION);
    /*
        End of tes3mp addition
    */

    /*
        Start of tes3mp change (minor)

        Because there is no need to print the commit hash again, only print OpenMW's version
    */
    Log(Debug::Info) << "OpenMW version " << v.mVersion;
    /*
        End of tes3mp change (minor)
    */
=======
    Debug::setupLogging(cfgMgr.getLogPath(), "OpenMW");
    Log(Debug::Info) << Version::getOpenmwVersionDescription();

    Settings::Manager::load(cfgMgr);

    MWGui::DebugWindow::startLogRecording();
>>>>>>> omw51

    engine.setGrabMouse(!variables["no-grab"].as<bool>());

    // Font encoding settings
    std::string encoding(variables["encoding"].as<std::string>());
    Log(Debug::Info) << ToUTF8::encodingUsingMessage(encoding);
    engine.setEncoding(ToUTF8::calculateEncoding(encoding));

    Files::PathContainer dataDirs(asPathContainer(variables["data"].as<Files::MaybeQuotedPathContainer>()));

    Files::PathContainer::value_type local(variables["data-local"]
                                               .as<Files::MaybeQuotedPathContainer::value_type>()
                                               .u8string()); // This call to u8string is redundant, but required to
                                                             // build on MSVC 14.26 due to implementation bugs.
    if (!local.empty())
        dataDirs.push_back(std::move(local));

    cfgMgr.filterOutNonExistingPaths(dataDirs);

    engine.setResourceDir(variables["resources"]
                              .as<Files::MaybeQuotedPath>()
                              .u8string()); // This call to u8string is redundant, but required to build on MSVC 14.26
                                            // due to implementation bugs.
    engine.setDataDirs(dataDirs);

    // fallback archives
    StringsVector archives = variables["fallback-archive"].as<StringsVector>();
    for (StringsVector::const_iterator it = archives.begin(); it != archives.end(); ++it)
    {
        engine.addArchive(*it);
    }

    StringsVector content = variables["content"].as<StringsVector>();
    if (content.empty())
    {
        Log(Debug::Error) << "No content file given (esm/esp, nor omwgame/omwaddon). Aborting...";
        return false;
    }
    engine.addContentFile("builtin.omwscripts");
    std::set<std::string> contentDedupe{ "builtin.omwscripts" };
    for (const auto& contentFile : content)
    {
        if (!contentDedupe.insert(contentFile).second)
        {
            Log(Debug::Error) << "Content file specified more than once: " << contentFile << ". Aborting...";
            return false;
        }
    }

    for (auto& file : content)
    {
        engine.addContentFile(file);
    }

    StringsVector groundcover = variables["groundcover"].as<StringsVector>();
    for (auto& file : groundcover)
    {
        engine.addGroundcoverFile(file);
    }

    if (variables.count("lua-scripts"))
    {
        Log(Debug::Warning) << "Lua scripts have been specified via the old lua-scripts option and will not be loaded. "
                               "Please update them to a version which uses the new omwscripts format.";
    }

    // startup-settings
    engine.setCell(variables["start"].as<std::string>());
    engine.setSkipMenu(variables["skip-menu"].as<bool>(), variables["new-game"].as<bool>());
    if (!variables["skip-menu"].as<bool>() && variables["new-game"].as<bool>())
        Log(Debug::Warning) << "Warning: new-game used without skip-menu -> ignoring it";

    // scripts
    engine.setCompileAll(variables["script-all"].as<bool>());
    engine.setCompileAllDialogue(variables["script-all-dialogue"].as<bool>());
<<<<<<< HEAD
    engine.setScriptConsoleMode (variables["script-console"].as<bool>());
    
    /*
        Start of tes3mp change (major)

        Clients should not be allowed to set any of these unilaterally in multiplayer, so
        disable them
    */
    /*
    engine.setStartupScript (variables["script-run"].as<Files::EscapeHashString>().toStdString());
    engine.setWarningsMode (variables["script-warn"].as<int>());
    engine.setScriptBlacklist (variables["script-blacklist"].as<Files::EscapeStringVector>().toStdStringVector());
    engine.setScriptBlacklistUse (variables["script-blacklist-use"].as<bool>());
    engine.setSaveGameFile (variables["load-savegame"].as<Files::EscapePath>().mPath.string());
    */
    /*
        End of tes3mp change (major)
    */
=======
    engine.setScriptConsoleMode(variables["script-console"].as<bool>());
    engine.setStartupScript(variables["script-run"].as<std::string>());
    engine.setWarningsMode(variables["script-warn"].as<int>());
    engine.setSaveGameFile(variables["load-savegame"].as<Files::MaybeQuotedPath>().u8string());
>>>>>>> omw51

    // other settings
    Fallback::Map::init(variables["fallback"].as<Fallback::FallbackMap>().mMap);
    engine.setSoundUsage(!variables["no-sound"].as<bool>());
    engine.setActivationDistanceOverride(variables["activate-dist"].as<int>());
    engine.enableFontExport(variables["export-fonts"].as<bool>());
    engine.setRandomSeed(variables["random-seed"].as<unsigned int>());

    /*
        Start of tes3mp addition

        Configure multiplayer using parsed variables
    */
    mwmp::Main::configure(&variables);
    /*
        End of tes3mp addition
    */

    return true;
}

namespace
{
    class OSGLogHandler : public osg::NotifyHandler
    {
        void notify(osg::NotifySeverity severity, const char* msg) override
        {
            // Copy, because osg logging is not thread safe.
            std::string msgCopy(msg);
            if (msgCopy.empty())
                return;

            Debug::Level level;
            switch (severity)
            {
                case osg::ALWAYS:
                case osg::FATAL:
                    level = Debug::Error;
                    break;
                case osg::WARN:
                case osg::NOTICE:
                    level = Debug::Warning;
                    break;
                case osg::INFO:
                    level = Debug::Info;
                    break;
                case osg::DEBUG_INFO:
                case osg::DEBUG_FP:
                default:
                    level = Debug::Debug;
            }
            std::string_view s(msgCopy);
            if (s.size() < 1024)
                Log(level) << (s.back() == '\n' ? s.substr(0, s.size() - 1) : s);
            else
            {
                while (!s.empty())
                {
                    size_t lineSize = 1;
                    while (lineSize < s.size() && s[lineSize - 1] != '\n')
                        lineSize++;
                    Log(level) << s.substr(0, s[lineSize - 1] == '\n' ? lineSize - 1 : lineSize);
                    s = s.substr(lineSize);
                }
            }
        }
    };
}

int runApplication(int argc, char* argv[])
{
    Platform::init();

#ifdef __APPLE__
    setenv("OSG_GL_TEXTURE_STORAGE", "OFF", 0);
#endif

    osg::setNotifyHandler(new OSGLogHandler());
    Files::ConfigurationManager cfgMgr;
    std::unique_ptr<OMW::Engine> engine = std::make_unique<OMW::Engine>(cfgMgr);

    engine->setRecastMaxLogLevel(Debug::getRecastMaxLogLevel());

    if (parseOptions(argc, argv, *engine, cfgMgr))
    {
        if (!Misc::checkRequiredOSGPluginsArePresent())
            return 1;

        engine->go();
    }

    return 0;
}

#ifdef ANDROID
extern "C" int SDL_main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
<<<<<<< HEAD
    /*
        Start of tes3mp addition

        Initialize the logger added for multiplayer
    */
    LOG_INIT(TimedLog::LOG_INFO);
    /*
        End of tes3mp addition
    */

    /*
        Start of tes3mp change (major)

        Instead of logging information in openmw.log, use a more descriptive filename
        that includes a timestamp
    */
    return wrapApplication(&runApplication, argc, argv, "/tes3mp-client-" + TimedLog::getFilenameTimestamp());
    /*
        End of tes3mp change (major)
    */
=======
    return Debug::wrapApplication(&runApplication, argc, argv, "OpenMW");
>>>>>>> omw51
}

// Platform specific for Windows when there is no console built into the executable.
// Windows will call the WinMain function instead of main in this case, the normal
// main function is then called with the __argc and __argv parameters.
#if defined(_WIN32) && !defined(_CONSOLE)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    return main(__argc, __argv);
}
#endif
