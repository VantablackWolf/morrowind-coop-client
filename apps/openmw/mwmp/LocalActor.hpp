#ifndef OPENMW_LOCALACTOR_HPP
#define OPENMW_LOCALACTOR_HPP

#include <components/openmw-mp/Base/BaseActor.hpp>

// BaseActor reaches RakNet, which reaches <windows.h>; see the header for why.
#include "WinAPIConflicts.hpp"

#include "../mwmechanics/creaturestats.hpp"
#include "../mwmechanics/drawstate.hpp"
#include "../mwmechanics/activespells.hpp"
#include "../mwworld/manualref.hpp"
#include "../mwworld/timestamp.hpp"

namespace mwmp
{
    class LocalActor : public BaseActor
    {
    public:

        LocalActor();
        virtual ~LocalActor();

        void update(bool forceUpdate);

        void updateCell();
        void updatePosition(bool forceUpdate);
        void updateAnimFlags(bool forceUpdate);
        void updateAnimPlay();
        void updateSpeech();
        void updateStatsDynamic(bool forceUpdate);
        void updateEquipment(bool forceUpdate, bool sendImmediately = false);
        void updateAttackOrCast();

        void sendEquipment();
        void sendSpellsActiveAddition(const std::string id, bool isStackingSpell, const MWMechanics::ActiveSpells::ActiveSpellParams& params);
        void sendSpellsActiveRemoval(const std::string id, bool isStackingSpell, MWWorld::TimeStamp timestamp);
        void sendDeath(char newDeathState);

        MWWorld::Ptr getPtr();
        void setPtr(const MWWorld::Ptr& newPtr);

        bool hasSentData;

    private:
        MWWorld::Ptr ptr;

        bool posWasChanged;

        /*
            Start of tes3mp addition

            The direction last put on the wire, so a change in it -- above all a return to
            zero when an actor stops -- is itself a reason to send. Without that, the other
            client keeps applying the last direction it was given and the actor animates a
            walk forever while standing still.
        */
        mwmp::records::Position sentDirection{};
        /*
            End of tes3mp addition
        */
        bool equipmentChanged;

        bool wasRunning;
        bool wasSneaking;
        bool wasForceJumping;
        bool wasForceMoveJumping;

        bool wasJumping;
        bool wasFlying;

        MWMechanics::DrawState lastDrawState;

        MWMechanics::DynamicStat<float> oldHealth;
        MWMechanics::DynamicStat<float> oldMagicka;
        MWMechanics::DynamicStat<float> oldFatigue;
    };
}

#endif //OPENMW_LOCALACTOR_HPP
