#ifndef OPENMW_PROCESSORACTORAUTHORITY_HPP
#define OPENMW_PROCESSORACTORAUTHORITY_HPP


#include "../ActorProcessor.hpp"
#include <components/detournavigator/navigator.hpp>
#include "apps/openmw/mwmp/Main.hpp"
#include "apps/openmw/mwmp/CellController.hpp"

namespace mwmp
{
    class ProcessorActorAuthority final: public ActorProcessor
    {
    public:
        ProcessorActorAuthority()
        {
            BPP_INIT(ID_ACTOR_AUTHORITY)
        }

        virtual void Do(ActorPacket &packet, ActorList &actorList)
        {
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Received %s about %s", strPacketID.c_str(), actorList.cell.getShortDescription().c_str());
            mwmp::CellController *cellController = Main::get().getCellController();

            /*
                Start of tes3mp change (major)

                Record who holds authority even when this cell is not loaded here yet.

                The authority guid and the ability to act on it are separate facts, and the
                merge treated them as one. On login the server loads the player's saved cell,
                assigns authority and broadcasts it while the client is still standing in the
                starting exterior, so isActiveWorldCell was false and the whole packet --
                including the identity of the authority -- was thrown away. No further grant
                is ever sent, so the client was left permanently believing nobody owned the
                cell.

                What that cost: a client that had earlier been granted authority kept its
                LocalActors, while also holding DedicatedActors for the same NPCs once the
                actor list arrived. Two controllers drove every NPC -- local AI moving them,
                network updates pulling them back -- which is what jittering and walking on
                the spot actually is.

                So the guid is recorded unconditionally, and only the part that genuinely
                needs a loaded cell -- creating LocalActors -- stays behind the check.
            */
            cellController->initializeCell(actorList.cell);
            mwmp::Cell *cell = cellController->getCell(actorList.cell);

            if (cell == nullptr)
            {
                LOG_APPEND(TimedLog::LOG_INFO, "%s", "- Ignoring it because that cell could not be initialized");
                return;
            }

            cell->setAuthority(guid);

            if (cellController->isActiveWorldCell(actorList.cell))
            {
                if (isLocal())
                {
                    LOG_APPEND(TimedLog::LOG_INFO, "- The new authority is me");
                    cell->uninitializeDedicatedActors();
                    cell->initializeLocalActors();
                    cell->updateLocal(true);

                    /*
                        Start of tes3mp change (major)

                        0.8.1 re-enabled DetourNavigator updates here, having disabled them in
                        CellController when no cells were initialized. 0.51 replaced the global
                        toggle with a scoped update guard -- there is nothing to re-enable, and
                        update() takes a guard as its second argument.

                        The nudge is kept: taking authority over a cell is exactly when the
                        navmesh around the player wants rebuilding.
                    */
                    MWBase::World* world = MWBase::Environment::get().getWorld();
                    world->getNavigator()->update(
                        world->getPlayerPtr().getRefData().getPosition().asVec3(), nullptr);
                    /*
                        End of tes3mp change (major)
                    */
                }
                else
                {
                    BasePlayer *player = PlayerList::getPlayer(guid);

                    if (player != 0)
                        LOG_APPEND(TimedLog::LOG_INFO, "- The new authority is %s", player->npc.mName.c_str());

                    cell->uninitializeLocalActors();
                }
            }
            else
            {
                LOG_APPEND(TimedLog::LOG_INFO, "%s",
                    "- Cell not loaded here yet; authority recorded, actors will follow when it loads");
            }
            /*
                End of tes3mp change (major)
            */
        }
    };
}

#endif //OPENMW_PROCESSORACTORAUTHORITY_HPP
