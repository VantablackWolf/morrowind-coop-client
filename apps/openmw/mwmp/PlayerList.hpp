#ifndef OPENMW_PLAYERLIST_HPP
#define OPENMW_PLAYERLIST_HPP

#include <components/esm3/custommarkerstate.hpp>
#include <components/esm3/loadnpc.hpp>
#include <components/openmw-mp/Base/BasePlayer.hpp>

#include "../mwmechanics/aisequence.hpp"

#include "../mwworld/manualref.hpp"

#include "DedicatedPlayer.hpp"

#include <map>
#include <RakNetTypes.h>

namespace MWMechanics
{
    class Actor;
}

namespace mwmp
{
    class PlayerList
    {
    public:

        static void update(float dt);

        static DedicatedPlayer *newPlayer(RakNet::RakNetGUID guid);

        static void deletePlayer(RakNet::RakNetGUID guid);
        static void cleanUp();

        static DedicatedPlayer *getPlayer(RakNet::RakNetGUID guid);
        static DedicatedPlayer *getPlayer(const MWWorld::Ptr &ptr);
        /*
            0.8.1 called this from a hook inside World::searchPtrViaActorId, because that
            function walked active cells and DedicatedPlayers were not findable there.

            0.51 removed searchPtrViaActorId entirely: WorldModel keeps a PtrRegistry keyed
            by ESM::RefNum, every reference is entered into it by registerPtr(), and
            DedicatedPlayer creates its Ptr through placeObject() -- so
            WorldModel::getPtr(refNum) already finds dedicated players and the hook is
            superseded rather than lost. This is kept because it is still the right way for
            mwmp code to ask "is this RefNum a dedicated player", which the registry cannot
            answer.
        */
        static DedicatedPlayer* getPlayer(ESM::RefNum actorRefNum);
        static std::vector<RakNet::RakNetGUID> getPlayersInCell(const ESM::Cell& cell);

        static bool isDedicatedPlayer(const MWWorld::Ptr &ptr);

        static void enableMarkers(const ESM::Cell& cell);

        static void clearHitAttemptActor(ESM::RefNum actorRefNum);

    private:

        static std::map<RakNet::RakNetGUID, DedicatedPlayer *> playerList;
    };
}

#endif //OPENMW_PLAYERLIST_HPP
