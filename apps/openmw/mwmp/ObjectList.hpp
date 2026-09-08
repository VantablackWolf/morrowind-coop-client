#ifndef OPENMW_OBJECTLIST_HPP
#define OPENMW_OBJECTLIST_HPP

#include <components/esm/refid.hpp>

#include <components/openmw-mp/Base/BaseObject.hpp>
#include "../mwgui/itemmodel.hpp"
#include "../mwworld/worldimp.hpp"
#include <RakNetTypes.h>

namespace mwmp
{
    class Networking;
    class ObjectList : public BaseObjectList
    {
    public:

        ObjectList();
        virtual ~ObjectList();

        void reset();

        void addBaseObject(BaseObject baseObject);
        mwmp::BaseObject getBaseObjectFromPtr(const MWWorld::Ptr& ptr);
        void addContainerItem(mwmp::BaseObject& baseObject, const MWWorld::Ptr& itemPtr, int itemCount, int actionCount);
        void addContainerItem(mwmp::BaseObject& baseObject, const MWGui::ItemStack& itemStack, int itemCount, int actionCount);
        void addContainerItem(mwmp::BaseObject& baseObject, const std::string itemId, int itemCount, int actionCount);
        void addEntireContainer(const MWWorld::Ptr& ptr);

        void editContainers(MWWorld::CellStore* cellStore);

        void activateObjects(MWWorld::CellStore* cellStore);
        void placeObjects(MWWorld::CellStore* cellStore);
        void spawnObjects(MWWorld::CellStore* cellStore);
        void deleteObjects(MWWorld::CellStore* cellStore);
        void lockObjects(MWWorld::CellStore* cellStore);
        void triggerTrapObjects(MWWorld::CellStore* cellStore);
        void scaleObjects(MWWorld::CellStore* cellStore);
        void setObjectStates(MWWorld::CellStore* cellStore);
        void moveObjects(MWWorld::CellStore* cellStore);
        void restockObjects(MWWorld::CellStore* cellStore);
        void rotateObjects(MWWorld::CellStore* cellStore);
        void animateObjects(MWWorld::CellStore* cellStore);
        void playObjectSounds(MWWorld::CellStore* cellStore);
        void setGoldPoolsForObjects(MWWorld::CellStore* cellStore);
        void activateDoors(MWWorld::CellStore* cellStore);
        void setDoorDestinations(MWWorld::CellStore* cellStore);
        void runConsoleCommands(MWWorld::CellStore* cellStore);
        void makeDialogueChoices(MWWorld::CellStore* cellStore);

        void setClientLocals(MWWorld::CellStore* cellStore);
        void setMemberShorts();

        void playMusic();
        void playVideo();

        void addAllContainers(MWWorld::CellStore* cellStore);
        void addRequestedContainers(MWWorld::CellStore* cellStore, const std::vector<BaseObject>& requestObjects);

        void addObjectGeneric(const MWWorld::Ptr& ptr);
        void addObjectActivate(const MWWorld::Ptr& ptr, const MWWorld::Ptr& activatingActor);
        void addObjectHit(const MWWorld::Ptr& ptr, const MWWorld::Ptr& hittingActor);
        void addObjectHit(const MWWorld::Ptr& ptr, const MWWorld::Ptr& hittingActor, const Attack hitAttack);
        void addObjectPlace(const MWWorld::Ptr& ptr, bool droppedByPlayer = false);
        void addObjectSpawn(const MWWorld::Ptr& ptr);
        void addObjectSpawn(const MWWorld::Ptr& ptr, const MWWorld::Ptr& master, std::string spellId, int effectId, float duration);
        void addObjectLock(const MWWorld::Ptr& ptr, int lockLevel);
        void addObjectDialogueChoice(const MWWorld::Ptr& ptr, std::string dialogueChoice);
        void addObjectMiscellaneous(const MWWorld::Ptr& ptr, unsigned int goldPool, float lastGoldRestockHour, int lastGoldRestockDay);
        void addObjectTrap(const MWWorld::Ptr& ptr, const ESM::Position& pos, bool isDisarmed);
        void addObjectScale(const MWWorld::Ptr& ptr, float scale);
        void addObjectSound(const MWWorld::Ptr& ptr, std::string soundId, float volume, float pitch);
        void addObjectState(const MWWorld::Ptr& ptr, bool objectState);
        void addObjectAnimPlay(const MWWorld::Ptr& ptr, std::string group, int mode);

        void addDoorState(const MWWorld::Ptr& ptr, MWWorld::DoorState state);
        void addMusicPlay(std::string filename);
        void addVideoPlay(std::string filename, bool allowSkipping);
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
        void addContainerItem(mwmp::BaseObject& baseObject, const ESM::RefId& itemId, int itemCount, int actionCount);
        void addObjectSound(const MWWorld::Ptr& ptr, const ESM::RefId& soundId, float volume, float pitch);
        void addObjectAnimPlay(const MWWorld::Ptr& ptr, const ESM::RefId& group, int mode);
        /*
            End of tes3mp addition
        */

        void addClientScriptLocal(const MWWorld::Ptr& ptr, int internalIndex, int value, mwmp::VARIABLE_TYPE variableType);
        void addClientScriptLocal(const MWWorld::Ptr& ptr, int internalIndex, float value);
        void addScriptMemberShort(std::string refId, int index, int shortVal);
        /*
            Start of tes3mp addition

            RefId-taking overloads -- see LocalPlayer.hpp for the reasoning. Converts once,
            here, so no engine call site has to know how a record id reaches the wire.
        */
        void addScriptMemberShort(const ESM::RefId& refId, int index, int shortVal);
        /*
            End of tes3mp addition
        */


        void sendObjectActivate();
        void sendObjectHit();
        void sendObjectPlace();
        void sendObjectSpawn();
        void sendObjectDelete();
        void sendObjectLock();
        void sendObjectDialogueChoice();
        void sendObjectMiscellaneous();
        void sendObjectRestock();
        void sendObjectTrap();
        void sendObjectScale();
        void sendObjectSound();
        void sendObjectState();
        void sendObjectAnimPlay();
        void sendDoorState();
        void sendMusicPlay();
        void sendVideoPlay();
        void sendClientScriptLocal();
        void sendScriptMemberShort();
        void sendContainer();
        void sendConsoleCommand();

    private:
        Networking *getNetworking();

    };
}

#endif //OPENMW_OBJECTLIST_HPP
