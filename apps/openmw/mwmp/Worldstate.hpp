#ifndef OPENMW_WORLDSTATE_HPP
#define OPENMW_WORLDSTATE_HPP

#include <components/esm/refid.hpp>

#include <components/openmw-mp/Base/BaseWorldstate.hpp>

namespace mwmp
{
    class Networking;
    class Worldstate : public BaseWorldstate
    {
    public:

        Worldstate();
        virtual ~Worldstate();

        void addRecords();

        bool containsExploredMapTile(int cellX, int cellY);
        void markExploredMapTile(int cellX, int cellY);

        void setClientGlobals();
        void setKills();
        void setMapExplored();
        void setWeather();

        /*
            Takes the protocol mirror: these cells arrive in a WorldstatePacket and are
            described by the wire, not by anything loaded. Converted to engine cells only
            where an engine call needs one.
        */
        void resetCells(std::vector<mwmp::records::Cell>* cells);

        void sendClientGlobal(std::string varName, int value, mwmp::VARIABLE_TYPE variableType);
        void sendClientGlobal(std::string varName, float value);
        /*
            Start of tes3mp addition

            RefId-taking overloads -- see LocalPlayer.hpp for the reasoning. Converts once,
            here, so no engine call site has to know how a record id reaches the wire.
        */
        void sendClientGlobal(const ESM::RefId& varName, int value, mwmp::VARIABLE_TYPE variableType);
        void sendClientGlobal(const ESM::RefId& varName, float value);
        /*
            End of tes3mp addition
        */

        void sendMapExplored(int cellX, int cellY, const std::vector<char>& imageData);
        void sendWeather(std::string region, int currentWeather, int nextWeather, int queuedWeather, float transitionFactor);

        void sendEnchantmentRecord(const ESM::Enchantment* enchantment);
        void sendPotionRecord(const ESM::Potion* potion, unsigned int quantity);
        void sendSpellRecord(const ESM::Spell* spell);

        void sendArmorRecord(const ESM::Armor* armor, std::string baseRefId = "");
        void sendBookRecord(const ESM::Book* book, std::string baseRefId = "");
        void sendClothingRecord(const ESM::Clothing* clothing, std::string baseRefId = "");
        void sendWeaponRecord(const ESM::Weapon* weapon, std::string baseRefId = "", unsigned int quantity = 1);

    private:

        std::vector<MapTile> exploredMapTiles;

        Networking *getNetworking();

    };
}

#endif //OPENMW_WORLDSTATE_HPP
