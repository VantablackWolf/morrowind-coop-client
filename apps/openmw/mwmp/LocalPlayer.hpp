#ifndef OPENMW_LOCALPLAYER_HPP
#define OPENMW_LOCALPLAYER_HPP

#include <components/esm/refid.hpp>

#include <components/openmw-mp/Base/BasePlayer.hpp>
#include "../mwmechanics/activespells.hpp"
#include "../mwworld/ptr.hpp"
#include "../mwworld/timestamp.hpp"
#include <RakNetTypes.h>

namespace MWWorld
{
    // 0.51's unified ESM3/ESM4 cell view; only referenced by const& here.
    class Cell;
}

namespace mwmp
{
    class Networking;
    class LocalPlayer : public BasePlayer
    {
    public:

        LocalPlayer();
        virtual ~LocalPlayer();

        time_t deathTime;
        bool receivedCharacter;

        bool isUsingBed;
        bool avoidSendingInventoryPackets;
        bool isReceivingQuickKeys;
        bool isPlayingAnimation;
        bool diedSinceArrestAttempt;
        unsigned int lastEnchantmentQuantity;

        void update();

        bool processCharGen();
        bool isLoggedIn();

        void updateStatsDynamic(bool forceUpdate = false);
        void updateAttributes(bool forceUpdate = false);
        void updateSkills(bool forceUpdate = false);
        void updateLevel(bool forceUpdate = false);
        void updateBounty(bool forceUpdate = false);
        void updateReputation(bool forceUpdate = false);
        void updatePosition(bool forceUpdate = false);
        void updateCell(bool forceUpdate = false);
        void updateEquipment(bool forceUpdate = false);
        void updateInventory(bool forceUpdate = false);
        void updateAttackOrCast();
        void updateAnimFlags(bool forceUpdate = false);

        void addItems();
        void addSpells();
        void addSpellsActive();
        void addJournalItems();
        void addTopics();

        void removeItems();
        void removeSpells();
        void removeSpellsActive();

        void die();
        void resurrect();

        void closeInventoryWindows();
        void updateInventoryWindow();

        void setCharacter();
        void setDynamicStats();
        void setAttributes();
        void setSkills();
        void setLevel();
        void setBounty();
        void setReputation();
        void setPosition();
        void setMomentum();
        void setCell();
        void setClass();
        void setEquipment();
        void setInventory();
        void setSpellbook();
        void setSpellsActive();
        void setCooldowns();
        void setQuickKeys();
        void setFactions();
        void setBooks();
        void setShapeshift();
        void setMarkLocation();
        void setSelectedSpell();

        void sendDeath(char newDeathState);
        void sendClass();
        void sendInventory();
        void sendItemChange(const mwmp::Item& item, unsigned int action);
        void sendItemChange(const MWWorld::Ptr& itemPtr, int count, unsigned int action);
        void sendItemChange(const std::string& refId, int count, unsigned int action);
        void sendStoredItemRemovals();
        void sendSpellbook();
        void sendSpellChange(std::string id, unsigned int action);
        void sendSpellsActive();
        void sendSpellsActiveAddition(const std::string id, bool isStackingSpell, const MWMechanics::ActiveSpells::ActiveSpellParams& params);
        void sendSpellsActiveRemoval(const std::string id, bool isStackingSpell, MWWorld::TimeStamp timestamp);
        void sendCooldownChange(std::string id, int startTimestampDay, float startTimestampHour);
        void sendQuickKey(unsigned short slot, int type, const std::string& itemId = "");
        void sendJournalEntry(const std::string& quest, int index, const MWWorld::Ptr& actor);
        void sendJournalIndex(const std::string& quest, int index);
        void sendFactionRank(const std::string& factionId, int rank);
        void sendFactionExpulsionState(const std::string& factionId, bool isExpelled);
        void sendFactionReputation(const std::string& factionId, int reputation);
        void sendTopic(const std::string& topic);
        void sendBook(const std::string& bookId);
        void sendWerewolfState(bool isWerewolf);
        void sendMarkLocation(const ESM::Cell& newMarkCell, const ESM::Position& newMarkPosition);
        // CellStore::getCell() returns MWWorld::Cell in 0.51.
        void sendMarkLocation(const MWWorld::Cell& newMarkCell, const ESM::Position& newMarkPosition);
        void sendSelectedSpell(const std::string& newSelectedSpellId);
        void sendItemUse(const MWWorld::Ptr& itemPtr, bool usingItemMagic = false, char currentDrawState = 0);
        void sendCellStates();

        void clearCellStates();
        void clearCurrentContainer();

        void storeCellState(const ESM::Cell& cell, int stateType);
        void storeCurrentContainer(const MWWorld::Ptr& container);
        void storeItemRemoval(const std::string& refId, int count);
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
        void sendItemChange(const ESM::RefId& refId, int count, unsigned int action);
        void sendSpellChange(const ESM::RefId& id, unsigned int action);
        void sendJournalIndex(const ESM::RefId& quest, int index);
        void sendBook(const ESM::RefId& bookId);
        void sendSelectedSpell(const ESM::RefId& newSelectedSpellId);
        void storeItemRemoval(const ESM::RefId& refId, int count);
        void sendJournalEntry(const ESM::RefId& quest, int index, const MWWorld::Ptr& actor);
        void sendTopic(const ESM::RefId& topicId);
        /*
            End of tes3mp addition
        */

        void storeLastEnchantmentQuantity(unsigned int quantity);

        void playAnimation();
        void playSpeech();

        MWWorld::Ptr getPlayerPtr();

    private:
        Networking *getNetworking();

    };
}

#endif //OPENMW_LOCALPLAYER_HPP
