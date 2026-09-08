#ifndef OPENMW_MWMP_MAIN
#define OPENMW_MWMP_MAIN

#include <components/esm/refid.hpp>

#include "../mwworld/ptr.hpp"
#include <boost/program_options.hpp>
#include <components/files/collections.hpp>

namespace mwmp
{
    class GUIController;
    class CellController;
    class LocalSystem;
    class LocalPlayer;
    class Networking;

    class Main
    {
    public:
        Main();
        ~Main();

        static void optionsDesc(boost::program_options::options_description *desc);
        static void configure(const boost::program_options::variables_map &variables);
        static bool init(std::vector<std::string> &content, Files::Collections &collections);
        static void postInit();
        static bool isInitialized();
        static void destroy();
        static const Main &get();
        static void frame(float dt);

        static bool isValidPacketScript(std::string scriptId);
        /*
            Start of tes3mp addition

            RefId-taking overloads.

            0.51 turned most record ids into ESM::RefId, so nearly every engine call site
            that feeds one of these functions now holds a RefId rather than a string. The
            conversion belongs here, at the seam, rather than repeated at ~20 call sites in
            files that have no other reason to know about the wire format.

            Each forwards through RefIdCompat::toWire(), which is the single definition of
            how a record id is spelled on the wire.
        */
        static bool isValidPacketScript(const ESM::RefId& scriptId);
        /*
            End of tes3mp addition
        */

        static bool isValidPacketGlobal(std::string globalId);

        static std::string getResDir();

        Networking *getNetworking() const;
        LocalSystem *getLocalSystem() const;
        LocalPlayer *getLocalPlayer() const;
        GUIController *getGUIController() const;
        CellController *getCellController() const;

        void updateWorld(float dt) const;

    private:
        static std::string resourceDir;
        static std::string address;
        static std::string serverPassword;
        Main (const Main&);
        ///< not implemented
        Main& operator= (const Main&);
        ///< not implemented
        static Main *pMain;
        Networking *mNetworking;
        LocalSystem *mLocalSystem;
        LocalPlayer *mLocalPlayer;

        GUIController *mGUIController;
        CellController *mCellController;

        std::string server;
        unsigned short port;
    };
}

#endif //OPENMW_MWMP_MAIN
